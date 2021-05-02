/* xpilot.c,v 1.3 1992/05/11 15:31:45 bjoerns Exp
 *
 *	This file is part of the XPilot project, written by
 *
 *	    Bjørn Stabell (bjoerns@stud.cs.uit.no)
 *	    Ken Ronny Schouten (kenrsc@stud.cs.uit.no)
 *
 *	Copylefts are explained in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <X11/Intrinsic.h>
#include "pack.h"
#include "bit.h"
#include "version.h"
#include "config.h"

#define ON(x)	      ( (strcasecmp(x, "true")==0) || (strcasecmp(x, "on")==0) )

#define MAX_LINE	80
#define MAX_NAME_LEN	11
#define NOT_SET		-1

Pack	req;
char	name[MAX_LINE];
char	base_addr[MAX_LINE];
int	socket_c, socket_i, pack_size;	    /* Contact socket, data socket */
bool	auto_connect=false, list_servers=false, auto_shutdown=false;
Display *disp;



double atod(char *str)
{
    double tmp;

    sscanf(str, "%lf", &tmp);
    return (tmp);
}

/*
 * NOTE (base_addr) that this routine only handles the first net address.  If
 * the machine has more than one connection, you'll have to specify the server
 * machine manually if it isn't on the first net.
*/
void *initaddr()
{
    struct hostent *hinfo;
    char tmp[MAX_LINE];
    int i;


    base_addr[0]='\0';
    gethostname(req.hostname, MAX_LINE);
    hinfo = gethostbyname(req.hostname);

    /* Get base address (cluster address?) */
    hinfo->h_addr;
    for (i=0; i<hinfo->h_length-1; i++) {
	sprintf(tmp, "%d.", (unsigned char)hinfo->h_addr[i]);
	strcat(base_addr, tmp);
    }

    /*
     * Get host's IP-address.
     */
#ifdef	USE_IP_NUMBER
    strcpy(req.hostname, base_addr);
    sprintf(tmp, "%d", (unsigned char)hinfo->h_addr[hinfo->h_length-1]);
    strcat(req.hostname, tmp);
#else
    strcpy(req.hostname, hinfo->h_name);
#endif
}


void get_defaults()
{
    char *str;


    /*
     * Defaults.
     */
    req.def.power=45.0; req.def.turnspeed=30.0; req.def.turnresistance=0.12;
    req.def.power_s=35.0;req.def.turnspeed_s=25.0;req.def.turnresistance_s=0.12;
    req.def.team=0;
    req.def.fuel3=500.0; req.def.fuel2=200.0; req.def.fuel1=100.0;
    req.def.instruments = SHOW_HUD_INSTRUMENTS | SHOW_HUD_HORIZONTAL;


    /*
     * Name
     */
    if (name[0]=='\0') {
	if ((disp!=NULL) && (str=XGetDefault(disp, "xpilot", "name"))!=NULL)
	    strcpy(name, str);
	else
	    strcpy(name, req.realname);
    }
    if (strlen(name)>MAX_NAME_LEN) {
	name[MAX_NAME_LEN]='\0';
	fprintf(stderr, "Name too long - chopped off: %s\n", name);
    }
    CAP_LETTER(name[0]);


    if (disp == NULL)
	return;

    /*
     * Control sensitivity.
     */
    str=XGetDefault(disp, "xpilot", "power");
    if (str)
	req.def.power = atod(str);
    str=XGetDefault(disp, "xpilot", "turnSpeed");
    if (str)
	req.def.turnspeed = atod(str);
    str=XGetDefault(disp, "xpilot", "turnResistance");
    if (str)
	req.def.turnresistance = atod(str);

    str=XGetDefault(disp, "xpilot", "altPower");
    if (str)
	req.def.power_s = atod(str);
    str=XGetDefault(disp, "xpilot", "altTurnSpeed");
    if (str)
	req.def.turnspeed_s = atod(str);
    str=XGetDefault(disp, "xpilot", "altTurnResistance");
    if (str)
	req.def.turnresistance_s = atod(str);


    /*
     * Misc. data, fuel limits on HUD.
     */
    str=XGetDefault(disp, "xpilot", "team");
    if (str)
	req.def.team = atoi(str);
    str=XGetDefault(disp, "xpilot", "fuelNotify");
    if (str)
	req.def.fuel3 = atod(str);
    str=XGetDefault(disp, "xpilot", "fuelWarning");
    if (str)
	req.def.fuel2 = atod(str);
    str=XGetDefault(disp, "xpilot", "fuelCritical");
    if (str)
	req.def.fuel1 = atod(str);
    
    /*
     * Instruments.
     */

    /* HUD */
    str=XGetDefault(disp, "xpilot", "showHUD");
    if (str) {
	if (ON(str)) {
	    SET_BIT(req.def.instruments, SHOW_HUD_INSTRUMENTS);
	} else {
	    CLR_BIT(req.def.instruments, SHOW_HUD_INSTRUMENTS);
	}
    }
    str=XGetDefault(disp, "xpilot", "verticalHUDLine");
    if (str) {
	if (ON(str)) {
	    SET_BIT(req.def.instruments, SHOW_HUD_VERTICAL);
	} else {
	    CLR_BIT(req.def.instruments, SHOW_HUD_VERTICAL);
	}
    }
    str=XGetDefault(disp, "xpilot", "horizontalHUDLine");
    if (str) {
	if (ON(str)) {
	    SET_BIT(req.def.instruments, SHOW_HUD_HORIZONTAL);
	} else {
	    CLR_BIT(req.def.instruments, SHOW_HUD_HORIZONTAL);
	}
    }

    /* FUEL */
    str=XGetDefault(disp, "xpilot", "fuelMeter");
    if (str) {
	if (ON(str)) {
	    SET_BIT(req.def.instruments, SHOW_FUEL_METER);
	} else {
	    CLR_BIT(req.def.instruments, SHOW_FUEL_METER);
	}
    }
    str=XGetDefault(disp, "xpilot", "fuelGauge");
    if (str) {
	if (ON(str)) {
	    SET_BIT(req.def.instruments, SHOW_FUEL_GAUGE);
	} else {
	    CLR_BIT(req.def.instruments, SHOW_FUEL_GAUGE);
	}
    }

    /* Misc. meters. */
    str=XGetDefault(disp, "xpilot", "turnSpeedMeter");
    if (str) {
	if (ON(str)) {
	    SET_BIT(req.def.instruments, SHOW_TURNSPEED_METER);
	} else {
	    CLR_BIT(req.def.instruments, SHOW_TURNSPEED_METER);
	}
    }
    str=XGetDefault(disp, "xpilot", "powerMeter");
    if (str) {
	if (ON(str)) {
	    SET_BIT(req.def.instruments, SHOW_POWER_METER);
	} else {
	    CLR_BIT(req.def.instruments, SHOW_POWER_METER);
	}
    }
}


void printfile(char *name)
{
    FILE *fp;
    char c;


    if ((fp=fopen(name, "r")) == NULL) {
/*	perror(name);	*/
	return;
    }

    while ((c=fgetc(fp)) && !feof(fp))
	putchar(c);

    fclose(fp);
}



int get_contact_message(char *server)
{
    bool readable = false;
    Pack pack;


    if (SocketReadable(socket_c)) {
	if (DgramReceiveAny(socket_c, (char *)&pack, sizeof(Pack)) == -1) {
	    perror("DgramReceiveAny");
	    exit(-1);
	}
	if (pack.type == CONTACT) {
	    readable = true;
	    strcpy(server, pack.string);
	}
    }
    
    return (readable);
}



Pack *get_info_message()
{
    static Pack pack;

    
    if (SocketReadable(socket_i)) {
	if (DgramReceiveAny(socket_i, (char *)&pack, sizeof(Pack)) == -1) {
	    perror("DgramReceiveAny");
	    exit(-1);
	} 
    } else return NULL;
    
    return &pack;
}



/*
 * This is the routine that interactively (if not auto_connect) prompts
 * the user on his/her next action.  Returns true if player joined this
 * server, or false if the player wants to have a look at the next server.
 */
bool Connect_to_server(char *server)
{
    bool contact;
    char c, str[MAX_LINE];
    Pack *pack;


    req.port = GetPortNum(socket_i);

 again:

    /*
     * Now, what do you want from the server?
     */
    if (!auto_connect) {
	printf("Server on %s. Enter command> ",
	       server);
	
	gets(str);
	c = str[0];
	CAP_LETTER(c);
    } else {
	if (list_servers)
	    c='S';
	else if (auto_shutdown)
	    c='D';
	else
	    c='J';
    }

    contact=true;
    switch (c) {

	/*
	 * Owner only commands:
	 */
    case 'K':
	req.type = KICK;
	printf("WARNING! Only owner can execute this operation.\n"
	       "Enter name of victim: ");
	gets(req.string);
	break;

    case 'M':				/* Send a message to server. */
	req.type = MESSAGE;
	printf("WARNING! Only the owner of the server can send a message.\n"
	       "Enter message: ");
	gets(req.string);
	break;

	/*
	 * Public commands:
	 */
    case 'N':				/* Next server. */
	contact=false;
	break;

    case 'S':				/* Report status. */
	req.type = REPORT_STATUS;
	break;

    case 'D':
	req.type = SHUTDOWN;
	if (!auto_shutdown) {
	    printf("WARNING! Only the owner of the server can send a message.\n"
		   "Enter reason: ");
	    gets(req.string);
	}
	break;

    case 'Q':
	exit (0);
	break;

    case 'L':
	req.type = LOCK;
	break;

    case '\0':
    case 'J':				/* Trying to enter game. */
	if (req.display[0] == '\0') {
	    fprintf(stderr, "ERROR: Display variable not set.\n");
	    if (auto_connect)
		exit (-1);
	    else
		goto again;
	}
	sprintf(str, XHOST_CMD, server);
	system(str);
	strcpy(req.string, name);
	req.type = ENTER_GAME;
	break;

    case '?':
    case 'H':				/* Help. */
    default:
	printf("CLIENT VERSION...: %s\n", TITLE);
	printf("Supported commands are:\n"
	       "H/? -	Help - this text.\n"
	       "N   -	Next server, skip this one.\n"
	       "S   -	list Status.\n"
	       "Q   -	Quit.\n"
	       "K   -	Kick a player.		     (only owner)\n"
	       "M   -	send a Message.		     (only owner)\n"
	       "L   -	Lock/unLock server access.   (only owner)\n"
	       "D   -	shutDown/cancel shutDown.    (only owner)\n"
	       "J or just Return enters the game.\n");
	goto again;
	break;
    }


    /*
     * Do you want to contact the server, or carry on?
     */
    if (contact) {
	if ((pack_size = DgramSend(socket_i, server, PORT_NR, 
				   (char *)&req, sizeof(Pack))) == -1) {
	    perror("Couldn't send request to server (DgramSend)");
	    exit(1);
	}

	if ((pack = get_info_message()) != NULL) {
	    if (!auto_connect || list_servers)
		printf("\nRESPONSE FROM %s: %s\n", server, pack->string);
	} else
	    perror("Could not get any answer from server");

	if (list_servers)	/* If listing servers, go to next one */
	    return false;

	if (auto_shutdown)
	    return true;

	if (auto_connect && pack->type!=ENTER_GAME) {
	    return false;			  /* return false. */
	}
	if (pack && pack->type == ENTER_GAME)
	    return true;
	else
	    goto again;
    } else
	return false;
}




/*
 * Oh glorious main(), without thee we cannot exist.
 */
int main(int argc, char *argv[])
{
    char machine[MAX_LINE];
    int i, team, packet_size = sizeof(Pack);
    struct passwd *pwent;
    bool connected = false;


    /*
     * Misc. init.
     */
    initaddr();

    if ((socket_i = CreateDgramSocket(0)) == -1) {
	perror("Could not create socket.");
	exit(-1);
    }

    if ((socket_c = CreateDgramSocket(0)) == -1) {
	perror("Could not create socket.");
	exit(-1);
    }

    machine[0]=name[0]='\0';


    /*
     * --- Setup the rest of the request (req.hostname already setup) ---
     */
    pwent = getpwuid(geteuid());
    strcpy(req.realname, pwent->pw_name);
    strcpy(req.display, getenv("DISPLAY"));
    req.port = GetPortNum(socket_c);
    req.type = CONTACT;
    team = NOT_SET;


    /*
     * --- Check commandline arguments ---
     */
    for(i=1; i<argc; i++) {
	if ((strcmp(argv[i], "-h")==0) || (strcmp(argv[i], "-help")==0)) {
	    printfile(HELPFILE);
	    exit (0);
	}
	if (strcmp(argv[i], "-name")==0) {
	    strcpy(name, argv[++i]);
	    continue;
	}
	if (strcmp(argv[i], "-team")==0) {
	    team = atoi(argv[++i]);
	    continue;
	}
	if (strcmp(argv[i], "-join")==0) {
	    auto_connect=true;
	    continue;
	}
	if (strcmp(argv[i], "-list")==0) {
	    list_servers=true;
	    auto_connect=true;
	    continue;
	}
	if (strcmp(argv[i], "-display")==0) {
	    strcpy(req.display, argv[++i]);
	    continue;
	}
	if (strcmp(argv[i], "-shutdown")==0) {
	    auto_shutdown=true;
	    auto_connect=true;
	    if (argc > i+1)
		strcpy(req.string, argv[++i]);
	    else
		strcpy(req.string, "Unknown reason.");
	    continue;
	}
	strcpy(machine, argv[i]);
    }

    if (strstr(req.display, "unix:0")!=NULL ||
	strstr(req.display, "local:0")!=NULL ||
	strcmp(req.display, ":0.0")==0 || strcmp(req.display, ":0")==0)
	sprintf(req.display, "%s:0.0", req.hostname);
	
    if ((disp=XOpenDisplay(req.display)) == NULL) {
	fprintf(stderr,
		"WARNING: Cannot connect to display \"%s\".\n"
		"	  As a consequence you will not be allowed to\n"
		"	  join the game.  Please set your DISPLAY variable,\n"
		"	  or use the -display <hostname:0.0> option.\n\n"
		"NOTE:	  You can still contact the server and give commands."
		"\n\n", req.display);
	req.display[0] == '\0';
    }


    /*
     * Get X defaults.
     */
    get_defaults();
    if (team != NOT_SET)
	req.def.team = team;

    if (req.def.team < 0 || req.def.team > 9) {
	req.def.team = 0;
	fprintf(stderr, "WARNING:  You can only spesify team from 0-9.\n"
		"Your will be on team 0.\n");
    }


    /*
     * --- Message of the Day ---
     */
    if (!auto_connect)
	printfile(MOTDFILE);
    if (list_servers)
	printf("LISTING AVAILABLE SERVERS:\n");

    /*
     * If sysadm's have complained alot, check for free machines before
     * letting the user play.  If room is crowded, don't let him play.
     */
#ifdef	LIMIT_ACCESS
    if (!list_servers && Is_allowed()==false)
	exit (-1);
#endif


    /*
     * --- Try to contact server ---
     */
    if (machine[0] != '\0') {		/* Server specified on command line? */
	SetTimeout(3, 0);
	DgramSend(socket_c, machine, PORT_NR, (char *)&req, sizeof(Pack));
	if (get_contact_message(machine))
	    connected = Connect_to_server(machine);

    } else {				/* Search after servers... */
	SetTimeout(10, 0);

	/*
	 * Got the IP address of name-server with the last part deleted, i.e.
	 *	129.242.16.110	  =>   129.242.16.
	 * Then search through all the machines in the 129.242.16 domain.
	 */
D(	printf("Sending packet to:\n"); )
	for (i=1;  i<255; i++) {
	    sprintf(machine, "%s%d", base_addr, i);
D(	    printf("%s\t", machine);	)
	    while (DgramSend(socket_c, machine, PORT_NR, (char *)&req,
			     sizeof(Pack)) < packet_size) {
		if (packet_size != -1) {
D(		    printf("Only transmitted %d bytes.\n", packet_size);    )
		    sleep(1);
		}
	    }
	}
D(	printf("\n");	)


	/* Hard coded search domains. */
#ifdef	UIT
	strcpy(base_addr, "129.242.16.");
	for (i=1;  i<255; i++) {
	    sprintf(machine, "%s%d", base_addr, i);
	    while (DgramSend(socket_c, machine, PORT_NR, (char *)&req,
			     sizeof(Pack)) == -1)
		sleep(1);
	}
#endif
#ifdef	CC
	strcpy(base_addr, "129.242.6.");
	for (i=1;  i<255; i++) {
	    sprintf(machine, "%s%d", base_addr, i);
	    while (DgramSend(socket_c, machine, PORT_NR, (char *)&req,
			     sizeof(Pack)) == -1)
		sleep(1);
	}
#endif

	/*
	 * Wait for answer.
	 */
	while (get_contact_message(machine)) {
	    if (connected = Connect_to_server(machine))
		break;
	}
    }

    /*
     * Cleanup.
     */
    if (socket_c!=-1)	    SocketClose(socket_c);
    if (socket_i!=-1)	    SocketClose(socket_i);
    
    exit (connected==true ? 0 : -1);
}
