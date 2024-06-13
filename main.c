/*
 * 	Display battery status in tray
 * 	2024 - Aminavy
 */

#include <gtk/gtk.h>
#include <sys/sysctl.h>
#include "batt.h"

#define DEFAULT_INTERVAL 5
#define DECHARGING 1
#define CHARGING 2

struct configuration {
	unsigned int update_interval;
};

#define EMPTY  1
#define CAUTION 10
#define LOW 50
#define	GOOD  80
#define FULL  100
       
static void create_tray_icon(const struct configuration *);
void read_battery_status(unsigned int *, int *, int *);
gboolean get_option(char, char *, int, char **, const char);

int main(int argc, char *argv[]) {
	char *str = (char *)malloc(sizeof(char)*6);
	struct configuration config;

	config.update_interval = DEFAULT_INTERVAL;

	if(argc > 1)
	{
		if(get_option('i', str, argc, argv, TRUE))
		{
			config.update_interval = atoi(str);
			free(str);
		}
		else if(get_option('v', NULL, argc, argv, FALSE))
		{
			printf("zbatt version: 0.1\n");
			free(str);
			exit(0);
		}
		else
		{
			printf("usage: zbatt [-i update interval] [-v]\n");
			free(str);
			exit(1);
		}
		
		//g_print("interval = %d\n", config.update_interval);
	}
	
	gtk_init(&argc, &argv);

	create_tray_icon(&config);

	gtk_main();

	return 0;
}

void update_battery_icon(GtkStatusIcon *icon, const struct batt_info *info)
{
	static char *status, tooltip_txt[20];

	if(info -> state == DECHARGING)
	{
		if(info -> life <= EMPTY)
			status = strdup("battery-empty");
		else if(info -> life <= CAUTION)
			status = strdup("battery-caution");
		else if(info -> life <= LOW)
			status = strdup("battery-low");
		else if(info -> life <= GOOD)
			status = strdup("battery-good");
		else if(info -> life <= FULL)
			status = strdup("battery-full");


	}
	else if(info -> state == CHARGING)
	{
		free(status);

		if(info -> life <= EMPTY)
			status = strdup("battery-empty-charging");
		else if(info -> life <= CAUTION)
			status = strdup("battery-caution-charging");
		else if(info -> life <= LOW)
			status = strdup("battery-low-charging");
		else if(info -> life <= GOOD)
			status = strdup("battery-good-charging");
		else if(info -> life < 100)
			status = strdup("battery-full-charging");
		else if(info -> life == 100)
			status = strdup("battery-full-charged");
	}
	else
		status = strdup("battery");

	
	g_print("status: %s\n", status);

	sprintf(tooltip_txt, "Charge: %d%% (%d:%d)", info -> life, info -> time/60, info -> time%60);

	gint size = gtk_status_icon_get_size(icon);

	GdkPixbuf *pix = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
			status,
			size,
			GTK_ICON_LOOKUP_USE_BUILTIN,
			NULL);
	gtk_status_icon_set_from_pixbuf(icon, pix);
	free(status);
	gtk_status_icon_set_tooltip_text(icon, tooltip_txt);
}

static void check_battery(GtkStatusIcon *icon, gpointer data)
{
	static unsigned int charge = 0;
	int time, state;
	static struct batt_info info;

	g_print("check_battery called!\n");

	read_battery_status(&charge, &time, &state);
	info.life = charge;
	info.time = time;
	info.state = state;
	update_battery_icon(icon, &info);
}

static void create_tray_icon(const struct configuration *config)
{
	static GtkStatusIcon *tray_icon;
	tray_icon = gtk_status_icon_new();

	gtk_status_icon_set_tooltip_text(tray_icon, "...");
	gtk_status_icon_set_visible(tray_icon, TRUE);

	check_battery(NULL, tray_icon);
	g_timeout_add_seconds(config -> update_interval, (GSourceFunc)check_battery, (gpointer)tray_icon);

	g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(check_battery), tray_icon);
}

void read_battery_status(unsigned int *life, int *time, int *state)
{
	size_t len;

	sysctlbyname("hw.acpi.battery.life", life, &len, NULL, 0);
	sysctlbyname("hw.acpi.battery.time", time, &len, NULL, 0);
	sysctlbyname("hw.acpi.battery.state", state, &len, NULL, 0);
	g_print("Battery life: %d\n", *life);
}

gboolean get_option(char op, char *var, int argc, char *argv[], const char flag)
{
	for(int i = 0; i<argc; i++)
	{
		if(*argv[i] == '-')
		{
			if(argv[i][1] == op)
			{
				i++;
				if(flag == FALSE) return TRUE;
				while(*argv[i] != '\0')
				{
					*var++ = *argv[i];
					++argv[i];
				}
				*var = '\0';
				g_print("input option: %s\n", var);
				return TRUE;
			}
		}
	}

	return FALSE;
}
