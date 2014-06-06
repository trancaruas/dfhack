#include <iostream>
#include <fstream>

#include "Core.h"
#include <Console.h>
#include <Export.h>
#include <PluginManager.h>

#include "DataDefs.h"
//#include "df/world.h"
#include "modules/Screen.h"
// #pragma once

#include "proto/screen.pb.h"


using namespace std;
using namespace DFHack;
using namespace df::enums;
using Screen::Pen;
using df::global::gps;

// #include "serv.h"
#define DFSERV_VERSION 2

// Here go all the command declarations...
// mostly to allow having the mandatory stuff on top of the file and commands on the bottom
command_result serv (color_ostream &out, std::vector <std::string> & parameters);

DFHACK_PLUGIN("serv");

// Mandatory init function. If you have some global state, create it here.
DFhackCExport command_result plugin_init ( color_ostream &out, std::vector <PluginCommand> &commands)
{
  //  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // Fill the command list with your commands.
  commands.push_back(PluginCommand(
				   "serv", "Do nothing yet, will serve df screen over tcp.",
				   serv, false,
				   "  Dump a current screen as screen.pb.dump file.\n"
				   "Example:\n"
				   "  serv\n"
				   "    Open server on port XXX.\n"
				   ));
  return CR_OK;
}

DFhackCExport command_result plugin_shutdown ( color_ostream &out )
{
    // You *MUST* kill all threads you created before this returns.
    // If everything fails, just return CR_FAILURE. Your plugin will be
    // in a zombie state, but things won't crash.
  /* google::protobuf::ShutdownProtobufLibrary(); */

  return CR_OK;
}

// Called to notify the plugin about important state changes.
// Invoked with DF suspended, and always before the matching plugin_onupdate.
// More event codes may be added in the future.

DFhackCExport command_result plugin_onstatechange(color_ostream &out, state_change_event event)
{
  /*
    switch (event) {
    case SC_GAME_LOADED:
        // initialize from the world just loaded
        break;
    case SC_GAME_UNLOADED:
        // cleanup
        break;
    default:
        break;
    }
  */

    return CR_OK;
}


// Whatever you put here will be done in each game step. Don't abuse it.
// It's optional, so you can just comment it out like this if you don't need it.
/*
DFhackCExport command_result plugin_onupdate ( color_ostream &out )
{
    // whetever. You don't need to suspend DF execution here.
    return CR_OK;
}
*/

void fillScreenProtobuf(color_ostream &out, std::vector <std::string> & parameters, Serv::Screen* scr) {
  int dimx = gps->dimx;
  int dimy = gps->dimy;
  Pen tile;

  scr->set_version(DFSERV_VERSION);
  scr->set_dimx(dimx);
  scr->set_dimy(dimy);

  out.print("Dumping screen with dimensions %dx%d v2\n", dimx, dimy);
  Serv::Pen* pen;
  for(int i = 0; i < dimx; i++) {
    for(int j = 0; j < dimy; j++) {
      tile = Screen::readTile(i, j);
      pen = scr->add_pen();

      //out.print("tm: %d", tile.tile_mode);
      switch(tile.tile_mode) {
      case Pen::CharColor: pen->set_tilemode(Serv::Pen::CHARCOLOR); break;
      case Pen::TileColor: pen->set_tilemode(Serv::Pen::TILECOLOR); break;
      default: pen->set_tilemode(Serv::Pen::ASIS); break;
      }

      pen->set_ch(tile.ch);
      pen->set_fg(tile.fg);
      pen->set_bg(tile.bg);
      pen->set_bold(tile.bold);
      pen->set_tile(tile.tile);
      pen->set_tile_fg(tile.tile_fg);
      pen->set_tile_bg(tile.tile_bg);
    }
  }
}


// A command! It sits around and looks pretty. And it's nice and friendly.
command_result serv (color_ostream &out, std::vector <std::string> & parameters)
{
  Serv::Screen scr;
  //out.print("dimx: %d, dimy: %d\n", gps->dimx, gps->dimy);

  // It's nice to print a help message you get invalid options
  // from the user instead of just acting strange.
  // This can be achieved by adding the extended help string to the
  // PluginCommand registration as show above, and then returning
  // CR_WRONG_USAGE from the function. The same string will also
  // be used by 'help your-command'.
  if (!parameters.empty())
    return CR_WRONG_USAGE;

  // Commands are called from threads other than the DF one.
  // Suspend this thread until DF has time for us. If you
  // use CoreSuspender, it'll automatically resume DF when
  // execution leaves the current scope.
  CoreSuspender suspend;

  //  out.print("Going to invoke fillScreenProtobuf\n");

  fillScreenProtobuf(out, parameters, &scr);
  // fstream output("scr1", ios::out | ios::trunc | ios::binary );
  // if( !scr.SerializeToOstream(&output) ) {
  //   out.print("Cannot serialize screen!\n");
  // }

  std::ofstream output("screen.pb.dump", std::ios_base::binary);
  output << scr.SerializeAsString();
  output.close();
	

  // Give control back to DF.
  return CR_OK;
}
