#include <cassert>
#include <iostream>
#include "ClinicRoom.h"
#include "LinkedQueue.h"
#include "Patient.h"

using namespace clinic;

namespace {

Patient makePatient(const std::string& id, Priority prio, int arrival, int duration) {
    Patient p;
    p.input.id = id;
    p.input.fullName = "Test " + id;
    p.input.age = 30;
    p.input.priority = prio;
    p.input.specialtyId = 1;
    p.input.ticketNumber = 1;
    p.input.arrivalTime = arrival;
    p.input.serviceDuration = duration;
    p.status = PatientStatus::NotArrived;
    p.assignedRoomId = -1;
    p.startTime = -1;
    p.finishTime = -1;
    return p;
}

void testLinkedQueue() {
    LinkedQueue q;
    assert(q.isEmpty());
    assert(q.size() == 0);
    assert(q.peek() == nullptr);
    assert(q.dequeue() == nullptr);

    Patient p1 = makePatient("BN01", Priority::Normal, 0, 5);
    Patient p2 = makePatient("BN02", Priority::Normal, 1, 5);
    Patient p3 = makePatient("BN03", Priority::Normal, 2, 5);
    Patient p4 = makePatient("BN04", Priority::Normal, 3, 5);
    Patient p5 = makePatient("BN05", Priority::Normal, 4, 5);

    // 1 nguoi -> lay ra -> rong
    q.enqueue(&p1);
    assert(!q.isEmpty() && q.size() == 1);
    assert(q.peek() == &p1);
    assert(q.dequeue() == &p1);
    assert(q.isEmpty() && q.size() == 0 && q.peek() == nullptr);

    // BN01 -> BN02 -> BN03, them BN04 roi lay lan luot
    q.enqueue(&p1);
    q.enqueue(&p2);
    q.enqueue(&p3);
    q.enqueue(&p4);
    assert(q.size() == 4);
    assert(q.dequeue() == &p1);
    assert(q.dequeue() == &p2);
    assert(q.dequeue() == &p3);
    assert(q.dequeue() == &p4);
    assert(q.isEmpty() && q.size() == 0);

    // Lay het roi them lai BN05
    q.enqueue(&p5);
    assert(q.size() == 1 && q.peek() == &p5);
    q.clear();
    assert(q.isEmpty() && q.size() == 0);
    assert(p5.input.id == "BN05"); // Patient van con nguyen, khong bi delete

    std::cout << "[PASS] testLinkedQueue\n";
}

void testClinicRoomBasic() {
    ClinicRoom room;
    room.configure(1, 1);
    assert(room.id() == 1);
    assert(room.specialtyId() == 1);
    assert(!room.isBusy());
    assert(room.waitingCount() == 0);

    Patient p0 = makePatient("BN00", Priority::Emergency, 0, 5);
    Patient p1 = makePatient("BN01", Priority::Preferential, 1, 5);
    Patient p2 = makePatient("BN02", Priority::Normal, 2, 5);

    // Bat uu tien: 0 & 1 vao heap, 2 vao FIFO
    room.enqueue(&p2, true);
    room.enqueue(&p1, true);
    room.enqueue(&p0, true);
    assert(room.normalQueueSize() == 1);
    assert(room.priorityQueueSize() == 2);
    assert(room.waitingCount() == 3);

    // Goi kham: lay tu heap truoc (p0 -> p1 -> p2)
    Patient* s1 = room.startNext(2, true);
    assert(s1 == &p0);
    assert(room.isBusy());
    assert(room.waitingCount() == 2); // khong tinh current
    assert(p0.status == PatientStatus::InService);
    assert(p0.assignedRoomId == 1);
    assert(p0.startTime == 2 && p0.finishTime == 7);

    // Phong dang ban thi khong goi nguoi moi
    assert(room.startNext(3, true) == nullptr);

    // Truoc gio xong (phut 6 < 7)
    assert(room.finishIfDue(6) == nullptr);

    // Dung gio xong (phut 7 == 7)
    assert(room.finishIfDue(7) == &p0);
    assert(p0.status == PatientStatus::Completed);
    assert(!room.isBusy());

    // Goi finish lan 2 khong tra cung nguoi
    assert(room.finishIfDue(7) == nullptr);

    // Reset don hang/current, khong xoa Patient
    room.reset();
    assert(room.waitingCount() == 0 && !room.isBusy());
    assert(p1.input.id == "BN01" && p2.input.id == "BN02");

    std::cout << "[PASS] testClinicRoomBasic\n";
}

void run3PatientsScenario(bool usePriority, int expectedWait[3]) {
    ClinicRoom room;
    room.configure(1, 1);

    Patient p[3] = {
        makePatient("BN01", Priority::Normal, 0, 5),
        makePatient("BN02", Priority::Normal, 1, 5),
        makePatient("BN03", Priority::Preferential, 2, 5)
    };
    p[0].input.ticketNumber = 1;
    p[1].input.ticketNumber = 2;
    p[2].input.ticketNumber = 3;

    for (int minute = 0; minute <= 15; ++minute) {
        room.finishIfDue(minute);
        for (int i = 0; i < 3; ++i) {
            if (p[i].input.arrivalTime == minute) {
                p[i].status = PatientStatus::Waiting;
                p[i].assignedRoomId = room.id();
                room.enqueue(&p[i], usePriority);
            }
        }
        if (!room.isBusy()) {
            room.startNext(minute, usePriority);
        }
    }

    for (int i = 0; i < 3; ++i) {
        int wait = p[i].startTime - p[i].input.arrivalTime;
        assert(wait == expectedWait[i]);
    }
}

void testThreePatientsExample() {
    int waitPriority[3] = {0, 9, 3};
    run3PatientsScenario(true, waitPriority);

    int waitFifo[3] = {0, 4, 8};
    run3PatientsScenario(false, waitFifo);

    std::cout << "[PASS] testThreePatientsExample (0/9/3 & 0/4/8)\n";
}

} // namespace

int main() {
    testLinkedQueue();
    testClinicRoomBasic();
    testThreePatientsExample();
    std::cout << "ALL TESTS FOR MEMBER A PASSED!\n";
    return 0;
}