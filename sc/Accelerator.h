#pragma once

#include "nvhls_pch.h"

#define SIZE 4      // Number of 16-bits data per transfer
#define TAPS_ 16     // Number of coefficients per FIR filter
#define TSTEP1 32   // Size of Inputs for First FIR
#define TSTEP2 48   // Size of Inputs for Second FIR

SC_MODULE(Accelerator)
{
public:
    sc_in_clk clk;
    sc_in<bool> rst;

    typedef sc_uint<64> Data;

    sc_out<sc_uint<8>> st_out;
    Connections::In<sc_uint<8>> ctrl_in;
    Connections::In<sc_uint<64>> w_in;
    Connections::In<sc_uint<64>> x_in;
    Connections::Out<sc_uint<64>> z_out;

    SC_HAS_PROCESS(Accelerator);
    Accelerator(sc_module_name name_) : sc_module(name_),
    st_out("st_out"), ctrl_in("ctrl_in"), w_in("w_in"), x_in("x_in"), z_out("z_out")
    {
        SC_THREAD (run);
        sensitive << clk.pos();
        NVHLS_NEG_RESET_SIGNAL_IS(rst);
    }

// private:
    // Separate data storage for the first and second FIR operations
    sc_uint<64> coef1[TSTEP1 / SIZE] = {0};
    sc_uint<64> coef2[TSTEP1 / SIZE] = {0};
    sc_uint<16> input_buffer1[TAPS_] = {0};
    sc_uint<16> input_buffer2[TAPS_] = {0};

    // void reset_logic() {
    //     if (rst.read() == SC_LOGIC_1) {
    //         st_out.write(0);
    //         ctrl_in.Reset();
    //         w_in.Reset();
    //         x_in.Reset();
    //         z_out.Reset();
    //     }
    // }

    void run()
    {   
        ctrl_in.Reset();
        w_in.Reset();
        x_in.Reset();
        z_out.Reset();

        sc_uint<64> output = 0;
        sc_uint<8> ctrl = 0;
        int coef_count1 = 0, coef_count2 = 0;

        st_out.write(ctrl);
        wait(); // Wait separates reset from operational behavior

        while (1)
        {
            if (!ctrl_in.Empty())
            {
                ctrl = ctrl_in.Pop();
                st_out.write(ctrl);
                coef_count1 = 0;
                coef_count2 = 0;
            }
            wait();

            if (ctrl == 0x01)
            {
                // First FIR Operation
                processFIR(coef1, input_buffer1, coef_count1, TSTEP1, output);
            }
            else if (ctrl == 0x02)
            {
                // Second FIR Operation
                processFIR(coef2, input_buffer2, coef_count2, TSTEP2, output);
            }

            wait();
        }
    }

    void processFIR(sc_uint<64> coef[], sc_uint<16> input_buffer[], int &coef_count, int tstep, sc_uint<64> &output)
    {
        // Load coefficients
        while (!w_in.Empty() && coef_count < tstep / SIZE)
        {
            coef[coef_count] = w_in.Pop();
            coef_count++;
            wait(); // Increase throughput but lower critpath
        }

        // Process inputs
        while (!x_in.Empty())
        {
            sc_uint<64> new_input = x_in.Pop();
            wait(); // Increase throughput but lower critpath

            // Update sliding window
            for (int j = 0; j < SIZE; ++j)
            {
                // Shift the buffer for sliding window
                for (int i = 0; i < TAPS_ - 1; ++i)
                {
                    input_buffer[i] = input_buffer[i + 1];
                }
                input_buffer[TAPS_ - 1] = new_input.range(j * 16 + 15, j * 16);

                // Compute FIR result
                sc_uint<16> result = 0;
                for (int k = 0; k < TAPS_; ++k)
                {
                    // Precompute constant indices
                    int coef_idx = k / SIZE;
                    int coef_bit_idx = (k % SIZE) * 16;

                    // Use precomputed indices for range extraction
                    sc_uint<16> coef_val = coef[coef_idx].range(coef_bit_idx + 15, coef_bit_idx);
                    result += input_buffer[k] * coef_val;
                }

                // Pack result into output
                output.range(j * 16 + 15, j * 16) = result;
            }

            z_out.Push(output);
        }
    }
};
