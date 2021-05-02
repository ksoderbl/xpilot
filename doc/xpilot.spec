#
# XPilot RPM spec file.
#
# $Id: xpilot.spec,v 5.12 2002/04/11 20:04:58 bertg Exp $
#

Summary:      A multiplayer Internet client/server 2D space game
Name:         xpilot
Version:      4.5.2
Release:      12
Group:        X11/Games/Video
Copyright:    GPL
Packager:     xpilot@xpilot.org
Url:          http://www.xpilot.org/
Source:       ftp://ftp.xpilot.org/pub/xpilot/xpilot-%{version}.tar.gz
Patch:        xpilot-%{version}.dif
Vendor:       XPilot
Autoreqprov:  yes

%description
XPilot is a multi-player 2D space game. Some features are borrowed
from classics like the Atari coin-ups Asteriods and Gravitar, and
the home-computer games Thrust (Commdore 64) and Gravity Force
(Commodore Amiga), but XPilot has many new aspects too.
Highlights include: 
  - True client/server based game; optimal speed for every player. 
  - Meta server with up to date information about servers hosting
    games around the world. 
  - A web of world-wide rating servers; compare your skills with
    pilots from all around the world, and climb the ladder of the
    world-wide rating list. 
  - 'Real physics'; particles of explosions and sparks from your
    engines all affect you if you're hit by them. This makes it
    possible to kill someone by blowing them into a wall with
    engine thrust or shock waves from explosions. 
  - Specialized editors for editing ship-shapes and maps. 
  - Game objective and gameplay adjustable through a number of
    options, specified on the commandline, in special option files,
    or in the map files. Examples of modes of the game: 
      * classical dogfight; equipped with only your gun, you have
	to rely on your maneuvering and tactical skills 
      * team; fight together, steal other teams's treasures (involves
	flying around with a ball in a string, much like in Thrust)
	and blow up their targets (which are, no doubt, heavily guarded) 
      * all out nuclear war; chose carefully between more than 15 weapon
	and defense systems to stay alive and annihilate your enemies 
      * race; make it through the deadly course before your opponents 
  - Adjustable gravity; adjustable by putting special attractors or
    deflectors in the world, or by adjusting the global gravity in
    various ways. 
  - Cannons and personalized and vengeful robot fighters give you a
    hard time. 
  - Watch your energy, and remember to 'dock' with a fuel station to
    refuel before it's too late. 
  - Defend your home base, or terrorize and steal someone else's. 
  - Equip your ship with the 15+ defense and weapon systems: afterburners,
    cloaking devices, sensors, transporters, extra cannons, mines and
    bombs, rockets (smarts, torpedos and nuclear), ECM, laser, extra
    tanks, autopilot etc. 
%prep
%setup
%patch

%build
xmkmf -a
make

%install
make install
make install.man
gzip -fnq9 /usr/X11R6/man/man6/xpilot.6
gzip -fnq9 /usr/X11R6/man/man6/xpilots.6
gzip -fnq9 /usr/X11R6/man/man6/xp-replay.6

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc doc/CREDITS
%doc doc/ChangeLog
%doc doc/FAQ
%doc doc/README*
%doc /usr/X11R6/man/man6/xpilot.6.gz
%doc /usr/X11R6/man/man6/xpilots.6.gz
%doc /usr/X11R6/man/man6/xp-replay.6.gz
/usr/X11R6/bin/xpilot
/usr/X11R6/bin/xpilots
/usr/X11R6/bin/xp-replay
/usr/X11R6/bin/xp-mapedit
/usr/X11R6/lib/xpilot/
