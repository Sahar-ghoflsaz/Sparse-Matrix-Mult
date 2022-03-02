HOST_BINARY := output_svector_host
DPU_BINARY := output_vec_dpu

HOST_SOURCES := host.c
DPU_SOURCES := dpu.c

.PHONY: all clean run

all: ${HOST_BINARY} ${DPU_BINARY}

clean:
	rm -f ${HOST_BINARY} ${DPU_BINARY}

run: all
	./${HOST_BINARY}

${HOST_BINARY}: ${DPU_BINARY} ${HOST_SOURCES}
	clang -O2 -DDPU_BINARY='"$(realpath ${DPU_BINARY})"' `dpu-pkg-config --cflags --libs dpu` ${HOST_SOURCES} -o $@

${DPU_BINARY}: ${DPU_SOURCES}
	clang --target=dpu-upmem-dpurte -O2 -DNR_TASKLETS=1 ${DPU_SOURCES} -o $@
