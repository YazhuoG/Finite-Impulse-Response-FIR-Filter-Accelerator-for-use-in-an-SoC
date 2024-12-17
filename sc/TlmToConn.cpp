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

#include "nvhls_pch.h"
#include "TlmToConn.h"
#include <string>
#include <iostream>
#include <iomanip>

#include <ac_reset_signal_is.h>

using namespace  std;


SC_HAS_PROCESS(TlmToConn);
TlmToConn::TlmToConn( sc_core::sc_module_name module_name)
  : sc_module (module_name),
    clk("clk", 1.0, SC_NS, 0.5, 0, SC_NS, true)

{
  target.register_b_transport(this, &TlmToConn::custom_b_transport);
  Connections::set_sim_clk(&clk);
  dut.clk(clk);
  driver.clk(clk);
  w_fifo.clk(clk);
  x_fifo.clk(clk);
  z_fifo.clk(clk);
  ctrl_fifo.clk(clk);
  dut.rst(reset_bar);
  driver.reset_bar(reset_bar);
  w_fifo.rst(reset_bar);
  x_fifo.rst(reset_bar);
  z_fifo.rst(reset_bar);
  ctrl_fifo.rst(reset_bar);

  dut.st_out(st_sig);
  driver.st_in(st_sig);

  dut.ctrl_in(ctrl_in);
  ctrl_fifo.deq(ctrl_in);
  driver.ctrl_out(ctrl_out);
  ctrl_fifo.enq(ctrl_out);

  dut.w_in(w_in);
  w_fifo.deq(w_in);
  driver.w_out(w_out);
  w_fifo.enq(w_out);

  dut.x_in(x_in);
  x_fifo.deq(x_in);
  driver.x_out(x_out);
  x_fifo.enq(x_out);

  dut.z_out(z_out);
  z_fifo.enq(z_out);
  driver.z_in(z_in);
  z_fifo.deq(z_in);

  SC_THREAD(run);
}

void TlmToConn::run()
{
    reset_bar = 1;
    wait(2, SC_NS);
    reset_bar = 0;
    wait(2, SC_NS);
    reset_bar = 1;

    while (1) {
      wait();
    }
}

void                                        
TlmToConn::custom_b_transport
 ( tlm::tlm_generic_payload &gp, sc_core::sc_time &delay )
{
  sc_dt::uint64      address   = gp.get_address();
  tlm::tlm_command   command   = gp.get_command();
  unsigned long      length    = gp.get_data_length();
  unsigned char     *dp        = gp.get_data_ptr();
  unsigned long long data      = reinterpret_cast<unsigned long long*>(dp)[0];
  sc_core::sc_time mem_delay(10,sc_core::SC_NS);

  tlm::tlm_generic_payload *gpp;

  cout << sc_core::sc_time_stamp() << " " << sc_object::name();
  switch (command) {
    case tlm::TLM_WRITE_COMMAND:
    {
      cout << " WRITE len:0x" << hex << length << " addr:0x" << address << endl;
      break;
    }
    case tlm::TLM_READ_COMMAND:
    {
      cout << " READ len:0x" << hex << length << " addr:0x" << address << endl; 
      break;
    }
    default:
    {
      cout << " ERROR Command " << command << " not recognized" << endl;
    } 
  }

  m_mutex.lock();
  driver.inq.push(&gp);
  wait(driver.outpeq.get_event());
  gpp=driver.outpeq.get_next_transaction();
  if (gpp!=&gp) {
    cout << sc_core::sc_time_stamp() << " " << sc_object::name() 
          << " ERROR: incomming payload pointer does not match outgoing payload pointer" << endl;
  }
  m_mutex.unlock();

  cout << sc_core::sc_time_stamp() << " " << sc_object::name() << " transaction complete" << endl;

  if (gp.get_address()==0x08 && command==tlm::TLM_WRITE_COMMAND) {
    if (data==(unsigned long long)0x0f) {
      cout << sc_core::sc_time_stamp() << ' ' << name() << " received exit signal" << endl;
      sc_stop();
    }
    // else if ((long long)regOut[1].read()==(long long)0x01) {
    //   for (int i = 0; i < Accelerator::numReg; i++) {
    //     cout << sc_core::sc_time_stamp() << ' ' << name() << " regOut[" << dec << i << "] = " << hex << regOut[i] << endl;
    //   }
    // }

  }

  return;     
}







