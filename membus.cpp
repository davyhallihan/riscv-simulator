#include "port.h"
#include "poll.h"
#include <queue>

class MEMBUS {
    public:
        PORT *CPU[2];
        PORT* INST1;
        PORT* INST2;
        PORT* MEM;

        std::queue<POLL*> Q_MEM;
        std::queue<POLL*> Q_INST1;
        std::queue<POLL*> Q_INST2;


        MEMBUS() {
            CPU[0] = new PORT();
            CPU[0]->msg = nullptr;
            CPU[1] = new PORT();
            CPU[1]->msg = nullptr;
            INST1 = new PORT();
            INST1->msg = nullptr;
            INST2 = new PORT();
            INST2->msg = nullptr;
            MEM = new PORT();
            MEM->msg = nullptr;
        }

        void cycle() {
            //POPULATE MEM/INST1/INST2 QUEUES
            if(CPU[0]->msg != nullptr) {
                if(CPU[0]->msg->dest == "MEM") {
                    Q_MEM.push(CPU[0]->msg);
                } else if(CPU[0]->msg->dest == "INST1") {
                    Q_INST1.push(CPU[0]->msg);
                } else if(CPU[0]->msg->dest == "INST2") {
                    Q_INST2.push(CPU[0]->msg);
                }
                CPU[0]->msg = nullptr;
            }
            if(CPU[1]->msg != nullptr) {
                if(CPU[1]->msg->dest == "MEM") {
                    Q_MEM.push(CPU[1]->msg);
                } else if(CPU[1]->msg->dest == "INST1") {
                    Q_INST1.push(CPU[1]->msg);
                } else if(CPU[1]->msg->dest == "INST2") {
                    Q_INST2.push(CPU[1]->msg);
                }
                CPU[1]->msg = nullptr;
            }
        
            //POPULATE PORTS
            if(MEM->msg == nullptr && !Q_MEM.empty()) {
                MEM->msg = Q_MEM.front();
                Q_MEM.pop();
            }
            if(INST1->msg == nullptr && !Q_INST1.empty()) {
                INST1->msg = Q_INST1.front();
                Q_INST1.pop();
            }
            if(INST2->msg == nullptr && !Q_INST2.empty()) {
                INST2->msg = Q_INST2.front();
                Q_INST2.pop();
            }

            if(CPU[0]->msg) { if(CPU[0]->msg->done) { CPU[0]->msg = nullptr; } }
            if(CPU[1]->msg) { if(CPU[1]->msg->done) { CPU[1]->msg = nullptr; } }
            
        }
};