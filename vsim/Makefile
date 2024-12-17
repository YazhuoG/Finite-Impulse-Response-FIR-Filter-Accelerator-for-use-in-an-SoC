CXXFLAGS = -DHLS_CATAPULT -DCONNECTIONS_ACCURATE_SIM -DSC_INCLUDE_DYNAMIC_PROCESSES

TOP_NAME = Accelerator
PROGNAME = fir
EXE_NAME=simv

SRC = $(wildcard ../sc/*.cpp)
OBJ = $(addprefix csrc/sysc/, $(addsuffix .o, $(basename $(SRC))))
HLS_DIR = ../hls/Catapult/$(TOP_NAME).v1
INCLUDES = -I../sc -I$(HLS_DIR) -I$(HLS_DIR)/scverify -I$(VCS_HOME)/include/systemc233 -I$(CATAPULT_HOME)/Mgc_home/shared/include -I$(MATCHLIB_HOME)/cmod/include
LIBS = -L$(RISCV)/vcs_lib64 -Wl,-rpath,$(RISCV)/vcs_lib64 -lspike

REBUILDABLES=$(OBJ) $(EXE_NAME)


$(EXE_NAME): $(OBJ) csrc/sysc/sysc_sim.o
	vcs  -full64 -sysc sc_main -timescale=1ps/1ps -LDFLAGS "$(LIBS)" -syscelab ../rocket_sim/$(PROGNAME).riscv
	stty sane

obj: $(OBJ) csrc/sysc/sysc_sim.o

csrc/sysc/ccs_wrapper/ccs_wrapper.o: $(HLS_DIR)/concat_sim_rtl.v $(HLS_DIR)/scverify/ccs_wrapper_$(TOP_NAME).v
	vlogan -full64 -psl "-sverilog" +v2k -sverilog $(HLS_DIR)/concat_sim_rtl.v
	vlogan -full64 -psl "-sverilog" -sysc=233 +v2k -sverilog -sc_model ccs_wrapper -sc_portmap $(HLS_DIR)/scverify/dut_v_ports.map $(HLS_DIR)/scverify/ccs_wrapper_$(TOP_NAME).v

csrc/sysc/sysc_sim.o: $(HLS_DIR)/scverify/sysc_sim.cpp csrc/sysc/ccs_wrapper/ccs_wrapper.o
	syscan -prec -full64 -cflags "$(CXXFLAGS) -DCCS_SYSC -DSC_USE_STD_STRING -DTOP_HDL_ENTITY=ccs_wrapper $(INCLUDES)" $(HLS_DIR)/scverify/sysc_sim.cpp

$(OBJ): $(SRC) $(wildcard *.h) csrc/sysc/sysc_sim.o
	syscan -prec -full64 -cflags "$(CXXFLAGS) -DCCS_SYSC -DSC_USE_STD_STRING -DTOP_HDL_ENTITY=ccs_wrapper $(INCLUDES)" $(SRC)


# build all .cpp files to .o files
csrc/sysc/%.o : %.cpp  csrc/sysc/sysc_sim.o
	syscan -prec -full64 -cflags "$(CXXFLAGS) -DCCS_SYSC -DSC_USE_STD_STRING -DTOP_HDL_ENTITY=ccs_wrapper $(INCLUDES)" $<

clean: 
	-rm -rf csrc simv.daidir AN.DB ucli.key simv simv_elab.daidir
	-rm simv_elab.daidir simv_elab.elab.error simv_elab.README

cleanelab:
	-rm -rf csrc/objs simv.daidir simv ucli.key

sim:
	./$(EXE_NAME) fir.riscv
	stty sane

gdb:
	gdb ./$(EXE_NAME)
