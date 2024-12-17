/*
 * Data Source for NVLabs Matchlib
 * (c) 2022-11-06 by W. Rhett Davis (rhett_davis@ncsu.edu)
 * 
 * Modified from the Source module in the testbench.cpp file
 * for the Adder example packaged with Matchlib.
 *
 * This module writes data to its output port using the (blocking) Push()
 * method.  A maximum of one value per cycle can be written.  The frequency
 * of writes can be reduced by passing a Pacer object to the constructor.
 * The values to write are specified in a required input file.
 * 
 * Input file format:
 *   The first line of the input file is a comment.
 *   Each additional line contains one time specifier and one value per line.
 *   The time specifier uses the format "time_mode time_val time_unit":
 *     time_mode  - either + or @
 *     time_val   - the time given as a floating point decimal
 *     time_unit  - can be fs, ps, ns, us, ms, or s
 *   If the time mode is +, then the module will wait for the specified 
 *   amount of additional simulation time to pass before sending the next value.
 *   If the time mode is @, then the module will wait until the exact
 *   simulation time specified until sending the next vaule. 
 *   Note that every line must contain a time and value to avoid an error.
 * Template arguments:
 *   typename T   -  Data-type for the output port
 *   typename cfg -  Configuration
 * 
 * Example Configuration:
 * 
 *   struct DestConfig {
 *     enum {
 *       verbose = 1,         // Set to 1 to print out messages 
 *                            //    to stdout before and after each Push() 
 *                            //    or 0 to execute silently
 *       exitWhenDone = 0,    // Set to 1 to exit the simulation when the
 *                            //    end of the file is reached
 *                            //    or 0 to allow the simulation to continue
 *     };
 *   };
 *  
 *  Constructor
 *  Source(sc_module_name name_, const Pacer& pacer_, const char *filename_ = NULL)
 *    arguments:
 *      name_        Instance name of the module
 *      pacer_       A Matchlib Pacer object used to throttle the behavior
 *      filename_    The name of a file with data to send
 * 
 */

#ifndef __SOURCE_H__
#define __SOURCE_H__

#include <string>
#include <fstream>

#include <testbench/Pacer.h>

template <typename T,typename cfg>
SC_MODULE (Source) {
    Connections::Out<T> x_out;

    sc_in <bool> clk;
    sc_in <bool> rst;
    const char *filename;
    bool verbose;
    Pacer pacer;

    void run()
    {
        x_out.Reset();
        pacer.reset();

        char time_mode;
        double time_val;
        std::string time_unit;
        sc_core::sc_time start_time;
        T x;
        
        // Wait for initial reset.
        wait(20.0, SC_NS);

        std::ifstream f(filename,ios::in);
        std::string comment;

        // Skip the first line, assume it is a comment
        if (f.good())
          std::getline(f,comment);

        wait();
	
        while(f.good()) {
            f >> time_mode >> time_val >> time_unit >> x >> std::ws;
            // std::cout << time_mode << ' ' << time_val << ' ' << time_unit << ' ' << x << std::endl;

            // Parse the time from the transaction file, store in start_time
            if (time_unit=="fs")
            { start_time=sc_core::sc_time(time_val,sc_core::SC_FS);  }
            else if (time_unit=="ps")
            { start_time=sc_core::sc_time(time_val,sc_core::SC_PS);  }
            else if (time_unit=="ns")
            { start_time=sc_core::sc_time(time_val,sc_core::SC_NS);  }
            else if (time_unit=="us")
            { start_time=sc_core::sc_time(time_val,sc_core::SC_US);  }
            else if (time_unit=="ms")
            { start_time=sc_core::sc_time(time_val,sc_core::SC_MS);  }
            else
            { start_time=sc_core::sc_time(time_val,sc_core::SC_SEC); }

            // If time_mode is '+', increment start_time by current time
            if (time_mode=='+')
            { start_time+=sc_core::sc_time_stamp(); }

            // Wait until the transaction start-time is reached
            if (sc_core::sc_time_stamp() < start_time)
            wait( start_time-sc_core::sc_time_stamp() );

            if (cfg::verbose) std::cout << "@" << sc_time_stamp() << "\t" << name() << " PUSH " << x << std::endl ;
            x_out.Push(x);            
            if (cfg::verbose) std::cout << "@" << sc_time_stamp() << "\t" << name() << " DONE" << std::endl ;

            wait();
            while (pacer.tic()) { 
                if (cfg::verbose) std::cout << "@" << sc_time_stamp() << "\t" << name() << " STALL" << std::endl ;
                wait(); 
            }
        }
        if (cfg::verbose) std::cout << "@" << sc_time_stamp() << "\t" << name() << " COMPLETED" << std::endl ;
        if (cfg::exitWhenDone) sc_stop();
    }

    SC_HAS_PROCESS(Source);

    Source(sc_module_name name_, const Pacer& pacer_, const char *filename_) : sc_module(name_),
    x_out("x_out"),
    clk("clk"),
    rst("rst"),
    filename(filename_),
    pacer(pacer_)
    {
        SC_THREAD(run);
        sensitive << clk.pos();
        NVHLS_NEG_RESET_SIGNAL_IS(rst);
    }
};

#endif
