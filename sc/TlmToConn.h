/*
 * TlmToConn module
 *
 * Created 2022-11-19 by W. Rhett Davis (rhett_davis@ncsu.edu)
 *
 * This module translates standard SystemC TLM transactions
 * into MatchLib Axi transactions.  This is accomplished by
 * creating instances similar to the MatchLib Master and
 * AxiSlaveToReg template classes.  The b_transport handler
 * for the TLM slave socket puts transactions into the
 * Master's queue and waits for it to drive the AXI
 * channels the connect to the device under test.
 */


#pragma once

#include "nvhls_pch.h"

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "Accelerator.h"
#include "TlmToConnDriver.h"
#include <mc_connections.h>
#ifdef TOP_HDL_ENTITY
// VCS/SC_VERIFY simulation
#include "sysc_sim.h"
#endif
 
 

class TlmToConn: public sc_core::sc_module
{
  public:  

  static const unsigned int buswidth=64;
  sc_dt::uint64  m_memory_size;
  sc_core::sc_mutex m_mutex;

  TlmToConn( sc_core::sc_module_name module_name);

  tlm_utils::simple_target_socket<TlmToConn,buswidth>  target;

  TlmToConnDriver driver{"driver"};

  sc_signal<sc_uint<8>> st_sig{"st_sig"};

  Connections::Combinational<sc_uint<64>> w_in{"w_in"},w_out{"w_out"},
    x_in{"x_in"},x_out{"x_out"},z_in{"z_in"},z_out{"z_out"};
  Connections::Combinational<sc_uint<8>> ctrl_in{"ctrl_in"},ctrl_out{"ctrl_out"};
  Connections::Fifo<sc_uint<64>,4> w_fifo{"w_fifo"}, x_fifo{"x_fifo"}, z_fifo{"z_fifo"};
  Connections::Fifo<sc_uint<8>,1> ctrl_fifo{"ctrl_fifo"};
 

#ifndef TOP_HDL_ENTITY
  CCS_DESIGN(Accelerator) dut{"dut"};
#else
  CCS_RTL::sysc_sim_wrapper dut{"dut"};
#endif

  sc_clock clk;
  sc_signal<bool> reset_bar{"reset_bar"};

  private:

  void run();	    

  void custom_b_transport
  ( tlm::tlm_generic_payload &gp, sc_core::sc_time &delay );

};


