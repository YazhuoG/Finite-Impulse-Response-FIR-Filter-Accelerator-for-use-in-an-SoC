
#include <stdio.h>

#define TAPS 16
#define TSTEP1 32
#define TSTEP2 48

// short coef[TAPS*2] = {
// #include "coef.inc"
// };

// short input[TSTEP1+TSTEP2] = {
// #include "input.inc"
// };


#include "expected.inc"

// CLOBBER is a compiler barrier
// Some resources on compiler barriers:  
//   https://youtu.be/nXaxk27zwlk  (original source, as far as I can tell)
//   https://stackoverflow.com/questions/37786547/enforcing-statement-order-in-c
//   https://preshing.com/20120625/memory-ordering-at-compile-time/
static void clobber() {
  asm volatile ("" : : : "memory");
}



int main( int argc, char* argv[] )
{

  // int n,m;
  volatile short *coef=(short *)0x60004000;
  volatile short *input=(short *)0x60002000;
  // // Uncomment the next line to avoid memory controller accesses
  // // short output[TSTEP1+TSTEP2];
  volatile short *output=(short *)0x60001000;
  // short error,total_error;

  // // First FIR filter operation
  // for (n=0; n<TSTEP1; n++) {
  //   output[n]=0;
  //   for (m=0; m<TAPS; m++) {
  //     if (n+m-TAPS+1 >= 0) {
  //       output[n]+=coef[m]*input[n+m-TAPS+1];
  //       // Uncomment the next line for detailed calculation
  //       // printf("cpu main n: %d coef[%d]: %d input[%d]: %d sum: %d\n",n,m,coef[m],n+m-TAPS+1,input[n+m-TAPS+1],output[n]);
  //     }
  //   }
  // }

  // // Second FIR filter operation
  // for (n=0; n<TSTEP2; n++) {
  //   output[TSTEP1+n]=0;
  //   for (m=0; m<TAPS; m++) {
  //     if (n+m-TAPS+1 >= 0) {
  //       output[TSTEP1+n]+=coef[TAPS+m]*input[TSTEP1+n+m-TAPS+1];
  //       // Uncomment the next line for detailed calculation
  //       // printf("cpu main n: %d coef[%d]: %d input[%d]: %d sum: %d\n",n,m,coef[TAPS+m],n+m-TAPS+1,input[TSTEP1+n+m-TAPS+1],output[TSTEP1+n]);
  //     }
  //   }
  // }

  // // Error check for both FIR filter operations
  // total_error=0;
  // for (n=0; n<(TSTEP1+TSTEP2); n++) {
  //   error=expected[n]-output[n];              // Error for this time-step
  //   total_error+=(error<0)?(-error):(error);  // Absolute value
  //   // Uncomment the next line for a detailed error check
  //   // printf("cpu main k: %d output: %d expected %d\n",n,output[n],expected[n]);
  // }

  // printf("cpu main FIR total error: %d\n",total_error);

  // The rest of this file contains tests to illustrate
  // transactions to and from the DMA and Accelerator.
  // Please do not include them in your solution.

  volatile long long *dma_st     = (volatile long long*)  0x70000000;
  volatile long long **dma_sr    = (volatile long long**) 0x70000010;
  volatile long long **dma_dr    = (volatile long long**) 0x70000018;
  volatile long long *dma_len    = (volatile long long*)  0x70000020;
  volatile long long *accel_st   = (volatile long long*)  0x70010000;
  volatile long long *accel_ctrl = (volatile long long*)  0x70010008;
  volatile long long *accel_w    = (volatile long long*)  0x70010010;
  volatile long long *accel_x    = (volatile long long*)  0x70010030;
  volatile long long *accel_z    = (volatile long long*)  0x70010050;

  // First FIR filter operation
  *accel_ctrl=1;
  *dma_sr=(volatile long long*)((long)coef & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)accel_w & 0x1fffffff);
  *dma_len=TSTEP1; // starts transfer
  clobber();

  *dma_sr=(volatile long long*)((long)input & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)accel_x & 0x1fffffff);
  *dma_len=TSTEP1; // starts transfer
  clobber();

  *dma_sr=(volatile long long*)((long)accel_z & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)output & 0x1fffffff);
  *dma_len=TSTEP1; // starts transfer 
  clobber();

  // First FIR filter operation
  // total_error=0;
  // for (n=0; n<TSTEP1; n++) {
  //   error=expected[n]-output[n];              // Error for this time-step
  //   total_error+=(error<0)?(-error):(error);  // Absolute value
  //   //printf("cpu main n: %d input: 0x%x output 0x%x\n",n,input[n],output[n]);
  // }
  // printf("cpu main DMA transfer 1 total error: %d\n",total_error);

  // Second FIR filter operation
  *accel_ctrl=2;
  *dma_sr=(volatile long long*)((long)coef + TSTEP1 & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)accel_w & 0x1fffffff);
  *dma_len=TSTEP1; // starts transfer
  clobber();

  *dma_sr=(volatile long long*)((long)input + 2*TSTEP1 & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)accel_x & 0x1fffffff);
  *dma_len=TSTEP2; // starts transfer
  clobber();

  *dma_sr=(volatile long long*)((long)accel_z & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)output + 2*TSTEP1 & 0x1fffffff);
  *dma_len=TSTEP2; // starts transfer 
  clobber();

  // Second FIR filter error check
  // total_error=0;
  // for (n=0; n<TSTEP2; n++) {
  //   error=expected[n]-output[n];              // Error for this time-step
  //   total_error+=(error<0)?(-error):(error);  // Absolute value
  //   //printf("cpu main n: %d input: 0x%x output 0x%x\n",n,input[n],output[n]);
  // }
  // printf("cpu main DMA transfer 2 total error: %d\n",total_error);

  *accel_ctrl=1;
  *dma_sr=(volatile long long*)((long)input + TSTEP1 & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)accel_x & 0x1fffffff);
  *dma_len=TSTEP1; // starts transfer
  clobber();
 
  *dma_sr=(volatile long long*)((long)accel_z & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)output + TSTEP1 & 0x1fffffff);
  *dma_len=TSTEP1; // starts transfer 
  clobber();

  // First FIR filter error check
  // total_error=0;
  // for (n=0; n<TSTEP1; n++) {
  //   error=expected[n]-output[n];              // Error for this time-step
  //   total_error+=(error<0)?(-error):(error);  // Absolute value
  //   //printf("cpu main n: %d input: 0x%x output 0x%x\n",n,input[n],output[n]);
  // }
  // printf("cpu main DMA transfer 3 total error: %d\n",total_error);

  // Second FIR filter operation
  *accel_ctrl=2;
  *dma_sr=(volatile long long*)((long)input + 2*TSTEP1 + TSTEP2 & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)accel_x & 0x1fffffff);
  *dma_len=TSTEP2; // starts transfer
  clobber();
 
  *dma_sr=(volatile long long*)((long)accel_z & 0x1fffffff);
  *dma_dr=(volatile long long*)((long)output + 2*TSTEP1 + TSTEP2 & 0x1fffffff);
  *dma_len=TSTEP2; // starts transfer 
  clobber();

  // Second FIR filter error check
  // total_error=0;
  // for (n=0; n<TSTEP2; n++) {
  //   error=expected[n]-output[n];              // Error for this time-step
  //   total_error+=(error<0)?(-error):(error);  // Absolute value
  //   //printf("cpu main n: %d input: 0x%x output 0x%x\n",n,input[n],output[n]);
  // }
  // printf("cpu main DMA transfer 4 total error: %d\n",total_error);

  // *dma_sr=(volatile long long*)((long)coef + TSTEP1 & 0x1fffffff);
  // *dma_dr=(volatile long long*)((long)accel_w & 0x1fffffff);
  // *dma_len=TSTEP1; // starts transfer
  // clobber();

  // *dma_sr=(volatile long long*)((long)input + 2*TSTEP1 & 0x1fffffff);
  // *dma_dr=(volatile long long*)((long)accel_x & 0x1fffffff);
  // *dma_len=TSTEP2; // starts transfer
  // clobber();
 
  // *dma_sr=(volatile long long*)((long)accel_z & 0x1fffffff);
  // *dma_dr=(volatile long long*)((long)output & 0x1fffffff);
  // *dma_len=TSTEP2; // starts transfer 
  // clobber();

  // // Second FIR filter operation
  // total_error=0;
  // for (n=0; n<TAPS; n++) {
  //   error=expected[n]-output[n];              // Error for this time-step
  //   total_error+=(error<0)?(-error):(error);  // Absolute value
  //   //printf("cpu main n: %d coef: 0x%x output 0x%x\n",n,coef[n],output[n]);
  // }
  // printf("cpu main DMA transfer 2 total error: %d\n",total_error);

  // *accel_ctrl=2;
  // // separate printf statement needed to give time for accel_ctrl to propagate to accel_st
  // printf("cpu main accel_ctrl write/read long long test error: ");
  // printf("%d\n",2-*accel_st); 
  // volatile short *sp=(volatile short *)accel_ctrl;
  // *sp=3;
  // printf("cpu main accel_ctrl write/read short test error: ");
  // printf("%d\n",3-*accel_st);
  // volatile char *cp=(volatile char *)accel_ctrl;
  // *cp=4;
  // printf("cpu main accel_ctrl write/read char test 1 error: ");
  // printf("%d\n",4-*accel_st);
  // // Sim does not hang on next line, becauase st is signal, not fifo
  // printf("cpu main accel_ctrl write/read char test 2 error: %d\n",4-*accel_st);

  // *accel_x=0x33;
  // *accel_x=0x34;
  // *accel_x=0x35;
  // *accel_x=0x36;

  // *dma_sr=(volatile long long*)((long)accel_z & 0x1fffffff);
  // *dma_dr=(volatile long long*)((long)output & 0x1fffffff);
  // *dma_len=32; // starts transfer 
  // clobber();

  // printf("cpu main accel_x write + accel_z DMA transfer test 1 error: %d\n",0x33-output[0]);
  // printf("cpu main accel_x write + accel_z DMA transfer test 2 error: %d\n",0x34-output[4]);
  // printf("cpu main accel_x write + accel_z DMA transfer test 3 error: %d\n",0x35-output[8]);
  // printf("cpu main accel_x write + accel_z DMA transfer test 4 error: %d\n",0x36-output[12]);


  // *accel_w=0x133;
  // *accel_w=0x134;
  // *accel_w=0x135;
  // *accel_w=0x136;
  // printf("cpu main accel_w write + accel_z read test 1 error: %d\n",0x133-*accel_z);
  // printf("cpu main accel_w write + accel_z read test 2 error: %d\n",0x134-*accel_z);
  // printf("cpu main accel_w write + accel_z read test 3 error: %d\n",0x135-*accel_z);
  // printf("cpu main accel_w write + accel_z read test 4 error: %d\n",0x136-*accel_z);
  // // The next line would cause sim to hang, because no data is in z_fifo
  // //printf("cpu main accel_x write + accel_z read test 5 error: %d\n",0x136-*accel_z); 


  *accel_ctrl=(volatile long long)0x0f;       // Exit

  return 0;
}
