/*
 * TlmToConnDriver module

 * Created 2022-11-19 by W. Rhett Davis (rhett_davis@ncsu.edu)
 * Modified from the Master.h file included with NVLabs MatchLib (2020)
 * This module is intended to be instantiated within the TlmToAxi module.
 * It may be possible to fold this functionality into TlmToAxi,
 * but new classes would likely need to be created to allow
 * sending/receiving transactions with the 
 * axi::axi4<>::read::chan<> and axi::axi4<>::write::chan<>
 * classes.
 */

#pragma once

#include <systemc.h>
#include <ac_reset_signal_is.h>

// #include <axi/axi4.h>
#include <nvhls_connections.h>
#include <hls_globals.h>

#include <queue>
#include <string>
#include <iomanip>
#include <sstream>
// #include <vector>
// #include <map>
// #include <math.h>
// #include <boost/assert.hpp>

// #include <boost/random/mersenne_twister.hpp>
// #include <boost/random/uniform_int_distribution.hpp>
// #include <algorithm>

class TlmToConnDriver : public sc_module {
 public:

  sc_in<bool> reset_bar;
  sc_in<bool> clk;

  std::queue <tlm::tlm_generic_payload*> inq;
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> outpeq;

  static const int DATA_WIDTH = 64;
  static const int bytesPerBeat = DATA_WIDTH >> 3;
  typedef sc_uint<DATA_WIDTH> Data;

  sc_in<sc_uint<8>> st_in;
  Connections::Out<sc_uint<8>> ctrl_out;
  Connections::Out<Data> w_out;
  Connections::Out<Data> x_out;
  Connections::In<Data> z_in;


  SC_CTOR(TlmToConnDriver)
      : reset_bar("reset_bar"), clk("clk"), 
        outpeq("outpeq"), st_in("st_in"), ctrl_out("ctrl_out"),
        w_out("w_out"), x_out("x_out"), z_in("z_in") {

    SC_THREAD(run);
    sensitive << clk.pos();
    async_reset_signal_is(reset_bar, false);
  }

  
 protected:
  void run() {

    tlm::tlm_generic_payload *gpp=NULL;
    unsigned char *dp=NULL;
    unsigned long  gplen=0, num_beats=0;
    unsigned int i;
    sc_dt::uint64  addr=0;
    unsigned long long *lldata;
    unsigned char *cdata;

    ctrl_out.Reset();
    w_out.Reset();
    x_out.Reset();
    z_in.Reset();

    wait();

    while (1) {
      wait();
      if (!inq.empty()) {
        gpp=inq.front();
        inq.pop();
        dp = gpp->get_data_ptr();
        addr=gpp->get_address();
        gplen=gpp->get_data_length();
        num_beats=(gplen % bytesPerBeat)?(gplen/bytesPerBeat+1):(gplen/bytesPerBeat);
        lldata=reinterpret_cast<unsigned long long*>(dp);
        cdata=reinterpret_cast<unsigned char*>(dp);
        if (gpp->get_command()==tlm::TLM_WRITE_COMMAND) {
          if ( ( (addr & 0x07F) == 0x08 ) && ( num_beats == 1 ) ) {
            cout << sc_time_stamp() << " " << name()
              << " WRITE addr=0x" << hex << addr << " length=0x" << gplen
              << " data=0x" << (int)(*cdata) << endl;
            ctrl_out.Push(*cdata);
            gpp->set_response_status( tlm::TLM_OK_RESPONSE );
            outpeq.notify(*gpp,SC_ZERO_TIME);             
          } else if ( ( (addr & 0x07F) == 0x10 ) ) {
            for (i=0 ; i<num_beats ; i++) {
              cout << sc_time_stamp() << " " << name()
                << " WRITE addr=0x" << hex << addr << " length=0x" << gplen
                << " data=0x" << lldata[i] << endl;
              w_out.Push(lldata[i]);
            }
            gpp->set_response_status( tlm::TLM_OK_RESPONSE );
            outpeq.notify(*gpp,SC_ZERO_TIME);             
          } else if ( ( (addr & 0x07F) == 0x30 ) ) {
            for (i=0 ; i<num_beats ; i++) {
              cout << sc_time_stamp() << " " << name()
                << " WRITE addr=0x" << hex << addr << " length=0x" << gplen
                << " data=0x" << lldata[i] << endl;
              x_out.Push(lldata[i]);
            }
            gpp->set_response_status( tlm::TLM_OK_RESPONSE );
            outpeq.notify(*gpp,SC_ZERO_TIME);             
          }
          else {
            cout << "\nError @" << sc_time_stamp() << " from " << name()
                << ": WRITE addr=0x" << hex << addr << " length=0x" << gplen
                << " not supported" << endl;
            gpp->set_response_status( tlm::TLM_COMMAND_ERROR_RESPONSE );
            outpeq.notify(*gpp,SC_ZERO_TIME);            
          }
        } else if (gpp->get_command()==tlm::TLM_READ_COMMAND) {
          if ( ( (addr & 0x07F) == 0x00 ) && ( num_beats == 1 ) ) {
            *lldata=0;  // Clear 64-bit data register
            *cdata=st_in.read();  // Assign the least-significant 8 bits
            cout << sc_time_stamp() << " " << name()
              << " READ addr=0x" << hex << addr << " length=0x" << gplen
              << " data=0x" << (int)(*cdata) << endl;
            gpp->set_response_status( tlm::TLM_OK_RESPONSE );
            outpeq.notify(*gpp,SC_ZERO_TIME);             
          } else if ( ( (addr & 0x07F) == 0x50 ) ) {
            for (i=0 ; i<num_beats ; i++) {
              lldata[i]=z_in.Pop();
              cout << sc_time_stamp() << " " << name()
                << " READ addr=0x" << hex << addr << " length=0x" << gplen
                << " data=0x" << lldata[i] << endl;
            }
            gpp->set_response_status( tlm::TLM_OK_RESPONSE );
            outpeq.notify(*gpp,SC_ZERO_TIME);             
          } else {
            cout << "\nError @" << sc_time_stamp() << " from " << name()
                << ": READ addr=0x" << hex << addr << " length=0x" << gplen
                << " not supported" << endl;
            gpp->set_response_status( tlm::TLM_COMMAND_ERROR_RESPONSE );
            outpeq.notify(*gpp,SC_ZERO_TIME);            
          }
        } else {
          cout << "\nError @" << sc_time_stamp() << " from " << name()
              << ": Command " << gpp->get_command() 
              << ", addr=0x" << hex << addr << " not recognized" << endl;
          gpp->set_response_status( tlm::TLM_COMMAND_ERROR_RESPONSE );
          outpeq.notify(*gpp,SC_ZERO_TIME);
        }
      }

    }

  }



};


