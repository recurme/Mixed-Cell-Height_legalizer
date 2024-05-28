#include <iostream>
#include "nall.h"



using std::cout;

int main(int argc, char *argv[]) {
    // Parsing lef/def files
    circuit ckt;
    init_log(LOG_NORMAL);
    ckt.doTech = true;
    ckt.read_files(argc, argv);
    ckt.setMaxXY();
    // NBLG algorithm
    Naller naller(ckt);
    naller.nall();
    // Write def file
    ckt.write_def();
    ckt.cal_hpwl();

    return 0;
}