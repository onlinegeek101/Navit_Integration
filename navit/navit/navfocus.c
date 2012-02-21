/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2008 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <glib.h>
#include "event.h"
#include "coord.h"
#include "navfocus.h"
#include "debug.h"
#include "callback.h"
#include "plugin.h"
#include "search.h"
#include "navit.h"
#include "point.h"
#include "route.h"
#include "mapset.h"
#include "attr.h"
#include "projection.h"
#include "item.h"
#include "navigation.h"
#include "map.h"

static GMainLoop *loop;

static void navfocus_main_loop_run(void)
{
	loop = g_main_loop_new (NULL, TRUE);
	if (g_main_loop_is_running (loop))
	{
		g_main_loop_run (loop);
	}
}


struct callback {
	void (*func)();
	int pcount;
	enum attr_type type;
	void *p[0];

};


struct navit {
	struct attr self;
	GList *mapsets;
	GList *layouts;
	struct gui *gui;
	struct layout *layout_current;
	struct graphics *gra;
	struct action *action;
	struct transformation *trans, *trans_cursor;
	struct compass *compass;
	struct route *route;
	struct navigation *navigation;
	struct speech *speech;
	struct tracking *tracking;
	int ready;
	struct window *win;
	struct displaylist *displaylist;
	int tracking_flag;
	int orientation;
	int recentdest_count;
	int osd_configuration;
	GList *vehicles;
	GList *windows_items;
	struct navit_vehicle *vehicle;
	struct callback_list *attr_cbl;
	struct callback *nav_speech_cb, *roadbook_callback, *popup_callback, *route_cb, *progress_cb;
	struct datawindow *roadbook_window;
	struct map *former_destination;
	struct point pressed, last, current;
	int button_pressed,moved,popped,zoomed;
	int center_timeout;
	int autozoom_secs;
	int autozoom_min;
	int autozoom_active;
	struct event_timeout *button_timeout, *motion_timeout;
	struct callback *motion_timeout_callback;
	int ignore_button;
	int ignore_graphics_events;
	struct log *textfile_debug_log;
	struct pcoord destination;
	int destination_valid;
	int blocked;
	int w,h;
	int drag_bitmap;
	int use_mousewheel;
	struct messagelist *messages;
	struct callback *resize_callback,*button_callback,*motion_callback,*predraw_callback;
	struct vehicleprofile *vehicleprofile;
	GList *vehicleprofiles;
	int pitch;
	int follow_cursor;
	int prevTs;
	int graphics_flags;
	int zoom_min, zoom_max;
	int radius;
	struct bookmarks *bookmarks;
	int flags;
		 /* 1=No graphics ok */
		 /* 2=No gui ok */
	int border;
	int imperial;
	struct attr **attr_list;
};

//! The navit_vehicule
struct navit_vehicle {
	int follow;
	/*! Limit of the follow counter. See navit_add_vehicle */
	int follow_curr;
	/*! Deprecated : follow counter itself. When it reaches 'update' counts, map is recentered*/
	struct coord coord;
	int dir;
	int speed;
	struct coord last; /*< Position of the last update of this vehicle */
	struct vehicle *vehicle;
	struct attr callback;
	int animate_cursor;
};

static void navfocus_main_loop_quit(void)
{
	if (loop) {
		g_main_loop_quit(loop);
		g_main_loop_unref(loop);
	}

}

struct event_watch {
	GIOChannel *iochan;
	guint source;
};

static gboolean
navfocus_call_watch(GIOChannel * iochan, GIOCondition condition, gpointer t)
{
	struct callback *cb=t;
	callback_call_3(cb,iochan,condition,t);
	return TRUE;
}

static struct event_watch *
navfocus_add_watch(void *fd, enum event_watch_cond cond, struct callback *cb)
{
	struct event_watch *ret=g_new0(struct event_watch, 1);
	int flags=0;
	ret->iochan = g_io_channel_unix_new(GPOINTER_TO_INT(fd));
	switch (cond) {
	case event_watch_cond_read:
		flags=G_IO_IN;
		struct callback {
			void (*func)();
			int pcount;
			enum attr_type type;
			void *p[0];

		};
		break;
	case event_watch_cond_write:
		flags=G_IO_OUT;
		break;
	case event_watch_cond_except:
		flags=G_IO_ERR|G_IO_HUP;
		break;
	}
	ret->source = g_io_add_watch(ret->iochan, flags, navfocus_call_watch, (gpointer)cb);
	return ret;
}

static void
navfocus_remove_watch(struct event_watch *ev)
{
	if (! ev)
		return;
	g_source_remove(ev->source);
	g_io_channel_unref(ev->iochan);
	g_free(ev);
}

struct event_timeout {
	guint source;
	struct callback *cb;
};

static gboolean
navfocus_call_timeout_single(struct event_timeout *ev)
{
	callback_call_0(ev->cb);
	g_free(ev);
	return FALSE;
}

static gboolean
navfocus_call_timeout_multi(struct event_timeout *ev)
{
	callback_call_0(ev->cb);
	return TRUE;
}


static struct event_timeout *
navfocus_add_timeout(int timeout, int multi, struct callback *cb)
{
	struct event_timeout *ret=g_new0(struct event_timeout, 1);
	ret->cb=cb;
	ret->source = g_timeout_add(timeout, multi ? (GSourceFunc)navfocus_call_timeout_multi : (GSourceFunc)navfocus_call_timeout_single, (gpointer)ret);

	return ret;
}

static void
navfocus_remove_timeout(struct event_timeout *ev)
{
	if (! ev)
		return;
	g_source_remove(ev->source);
	g_free(ev);
}

struct event_idle {
	guint source;
	struct callback *cb;
};

static gboolean
navfocus_call_idle(struct event_idle *ev)
{
	callback_call_0(ev->cb);
	return TRUE;
}

static struct event_idle *
navfocus_add_idle(int priority, struct callback *cb)
{
	struct event_idle *ret=g_new0(struct event_idle, 1);
	ret->cb=cb;
	ret->source = g_idle_add_full(priority+100, (GSourceFunc)navfocus_call_idle, (gpointer)ret, NULL);
	return ret;
}

static void
navfocus_remove_idle(struct event_idle *ev)
{
	if (! ev)
		return;
	g_source_remove(ev->source);
	g_free(ev);
}

static void
navfocus_call_callback(struct callback_list *cb)
{
/*
 Idea for implementation:
 Create a pipe then use add_watch
 add callback to a queue
 from here write to the pipe to wakeup the pool
 then from the gui thread process the callback queue
*/
}

static struct event_methods navfocus_methods = {
	navfocus_main_loop_run,
	navfocus_main_loop_quit,
	navfocus_add_watch,
	navfocus_remove_watch,
	navfocus_add_timeout,
	navfocus_remove_timeout,
	navfocus_add_idle,
	navfocus_remove_idle,
	navfocus_call_callback,
};

struct event_priv {
};
#include "RouteInputAnalyzer.h"
#include "OverlayRendererControl.h"
#include "ImageCreation.h"


int hasGottenUserInput = 0;
void calculateHeading(struct navigation* this_)
{
	if(hasGottenUserInput)
	{
		//fprintf(stdout,"Heading Updated\n");
		char *results = g_new(char,1000);
		headingUpdate(results,this_);
		//fprintf(stdout,"SPeed %s\n",results);
		fflush(stdout);

	}
}


struct map_rect_priv {
	struct navigation *nav;
	struct navigation_command *cmd;
	struct navigation_command *cmd_next;
	struct navigation_itm *itm;
	struct navigation_itm *itm_next;
	struct navigation_itm *cmd_itm;
	struct navigation_itm *cmd_itm_next;
	struct item item;
	enum attr_type attr_next;
	int ccount;
	int debug_idx;
	struct navigation_way *ways;
	int show_all;
	char *str;
};


void getToNextNode(struct map_rect * mr)
{
	if(mr)
		{
			if(mr->priv->itm == NULL)
			{
				mr->priv->itm = mr->priv->itm_next;
			}
			while(mr->priv->itm != NULL && mr->priv->cmd == NULL)
			{
				//mr->priv->itm = mr->priv->itm_next;
				mr->priv->cmd = mr->priv->cmd_next;
			}
		}

}


static int
angle_delta(int angle1, int angle2)
{
	int delta=angle2-angle1;
	if (delta <= -180)
		delta+=360;
	if (delta > 180)
		delta-=360;
	return delta;
}


struct navigation {
	struct route *route;
	struct map *map;
	struct item_hash *hash;
	struct vehicleprofile *vehicleprofile;
	struct navigation_itm *first;
	struct navigation_itm *last;
	struct navigation_command *cmd_first;
	struct navigation_command *cmd_last;
	struct callback_list *callback_speech;
	struct callback_list *callback;
	struct navit *navit;
	struct speech *speech;
	int level_last;
	struct item item_last;
	int turn_around;
	int turn_around_limit;
	int distance_turn;
	struct callback *route_cb;
	int announce[route_item_last-route_item_first+1][3];
	int tell_street_name;
	int delay;
	int curr_delay;
};


FILE * jons_data;

void headingUpdate(char * heading,struct navigation * nav)
{
		struct map * map=NULL;
		map_rect * mr=NULL;
		map = navigation_get_map(nav);
		if(map)
			mr = map_rect_new(map,NULL);
		struct map_rect_priv *nice= mr->priv;
		if(&mr != NULL && mr->priv->itm_next != NULL)
		{
			navigation_itm * current_item = mr->priv->itm_next;
			getToNextNode(mr);
			char * nodeName = current_item->way.name1;
			if(nodeName == NULL)
			{
				nodeName = current_item->way.name2;
			}
			//printf("\tNext Road Angle: %d\n",current_item->angle_end);
			char * turn_dir;
			int prev_angle = 0;
			if(current_item->prev != NULL)
			{
				prev_angle = current_item->prev->way.angle2;
			}
			int turn_num;
			int choice=nav->cmd_first->delta;
			if( choice > 0)
			{
				turn_dir = "Right";
				turn_num = 100;
			}
			else
			{
				turn_dir = "Left";
				turn_num = -100;
			}
			//printf("\tNext Turn Direction: %s\n",turn_dir);
			int length = current_item->dest_length-mr->priv->cmd_next->itm->dest_length;
			//printf("\tDistance to Next Road: %d Meters\n",length);
			printf("\tCurrent Speed Limit: %d MPH\n",current_item->speed);
			sprintf(heading,"Next Node %s Distance: %d Turn %s\n",nodeName,length,turn_dir);
			//updateRouteData(int velocity, int road_angle, int distance, int turn_angle, int x1, int y1, int x2, int y2)
			int a = updateRouteData(nav->navit->vehicle->speed, 0, length, turn_num,0, 0, 0, 0);
			if(jons_data == NULL)
			{
				jons_data = fopen("jons_data","w+");
			}
			
			if(jons_data == NULL)
			{
				printf("Unable to Write Jon's Data\n");
			}
			else
			{
				fprintf(jons_data,"%d,%d,%d,%d",nav->navit->vehicle->speed, 0, length, turn_num);
			}
			printf(" Number %d", current_item->angle_end);
			processImageData();
		}
}

void readUserInput(void)
{
	//char buffer[1000];
	//fgets(&buffer,1000,0);
	printf("Yata! ");//%s",&buffer);
}

void setPosition(char * town,char* street)
{
	printf("Looking Up Address...%s,%s\n",town,street);
	struct coord result;
	struct navit * this_ = getNavit();
	search(town,street,&result);
	struct pcoord dest;
	dest.pro = projection_mg;
	dest.x = result.x;
	dest.y = result.y;
	if(&result != NULL)
	{
		initializeSystem(1600, 900, "red");
		navit_set_destination(this_,&dest,street,1);
	}
}


void search(char * town_name,char* street_name, struct coord* result)
{

	struct attr country;
	country.type = attr_country_all;
	country.u.str = "United States";
	search_list * list = search_list_new(navit_get_mapset(getNavit()));
	search_list_search(list, &country, 0);
	search_list_result * res;
	while((res=search_list_get_result(list)));
	struct attr town;
	town.type = attr_town_name;
	town.u.str = town_name;
	printf("Searching Towns for ... %s\n",town_name);
	search_list_search(list, &town,0);
	while((res=search_list_get_result(list)))
	{
		printf("\t");
		printf(res->town->common.town_name);
		printf("\n");
	}
	struct attr str;
	str.type = attr_street_name;
	str.u.str = street_name;
	printf("Searching Streets for ... %s\n",street_name);
	search_list_search(list, &str,1);
	while((res=search_list_get_result(list)))
	{
		printf(res->street->name);
		struct coord_geo geoPoint;
		struct coord cor;
		cor.x = res->street->common.c->x;
		cor.y = res->street->common.c->y;
		transform_to_geo(projection_mg,&cor,&geoPoint);
		printf(" X Coord: %f",geoPoint.lat);
		printf(" Y Coord: %f\n",geoPoint.lng);
		result->x = cor.x;
		result->y = cor.y;
	}
}

gboolean
readUserInput_two(GIOChannel *ioch, GIOCondition cond, gpointer data)
{
  int bytes_read;
  int length = 1000;
  GString*    g_string_new = g_new0(GString,1);

  GIOStatus out = G_IO_STATUS_NORMAL;
  do {
	  out = g_io_channel_read_line_string(ioch, g_string_new, &length, NULL);
    if (out != G_IO_STATUS_ERROR) {
    	char * temp = strchr(g_string_new->str,'\n');
    	if(temp != NULL)
    	{
    		*temp = '\0';
    	}
    	char * town = strtok(g_string_new->str,":");
    	char * street = strtok(NULL,":");
    	if(street != NULL)
    	{
    		printf("Town: %s\n",town);
    		printf("Street: %s\n",street);
    		setPosition(town,street);
    		hasGottenUserInput = 1;

    	}
      //process_char(chr);
      //if (pref.diag_comm) printf("%02x\n", chr);
    }
  } while (out == G_IO_STATUS_AGAIN);
  return TRUE;
}

static struct event_priv*
navfocus_new(struct event_methods *meth)
{
	printf("Waiting for User Input!! Town:Street Please!\n");
	*meth=navfocus_methods;
	GIOChannel * ioch = g_new0(GIOChannel,1);
	GIOCondition * cond = g_new0(GIOCondition,1);
	gpointer * data = g_new0(gpointer,1);
	void* io = ioch;
	void* co = cond;
	void* da = data;
	void ** p = malloc(3*sizeof(void*));
	p[0] = io;
	p[1] = co;
	p[2] = da;
	struct callback * cb = callback_new(readUserInput_two,0, NULL);
	navfocus_add_watch(0,event_watch_cond_read, cb);
	return (struct event_priv *)navfocus_new;
}

void
navfocus_init(void)
{
	plugin_register_event_type("glib", navfocus_new);
}

