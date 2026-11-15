#include "Patient.h"
#include <limits>
#include <stdexcept>

namespace clinic {
namespace {
bool validText(const std::string& value) {
    return value.find_first_not_of(" \t\r\n") != std::string::npos &&
           value.find_first_of("|\r\n") == std::string::npos;
}
}

Priority classifyPriority(int age, bool emergency) {
    if (emergency) return Priority::Emergency;
    return age < 6 || age >= 60 ? Priority::Preferential : Priority::Normal;
}

bool validatePatientInput(const PatientInput& input, int closingTime,
                          std::string& error) {
    error.clear();
    if (!validText(input.id) || !validText(input.fullName))
        error = "Ma va ho ten phai co noi dung, khong chua | hoac xuong dong.";
    else if (input.age < 0 || input.age > 120)
        error = "Tuoi phai trong [0, 120].";
    else if (input.specialtyId < 1 || input.specialtyId > kSpecialtyCount)
        error = "Khoa phai trong [1, 3].";
    else if (input.ticketNumber <= 0)
        error = "Ticket phai duong.";
    else if (closingTime <= 0 || closingTime == std::numeric_limits<int>::max() ||
             input.arrivalTime < 0 || input.arrivalTime >= closingTime)
        error = "Phut den phai nam trong gio nhan hop le.";
    else if (input.serviceDuration <= 0 ||
             input.serviceDuration > std::numeric_limits<int>::max() - 1 - input.arrivalTime)
        error = "Thoi luong phai duong va khong lam tran dong ho.";
    else if (input.priority != classifyPriority(input.age, input.priority == Priority::Emergency))
        error = "Muc uu tien khong phu hop tuoi/cap cuu.";
    return error.empty();
}

void resetPatientState(Patient& patient) {
    patient.status = PatientStatus::NotArrived;
    patient.assignedRoomId = -1;
    patient.startTime = -1;
    patient.finishTime = -1;
}

int waitingTime(const Patient& patient) {
    if (patient.startTime < 0 || patient.input.arrivalTime < 0 ||
        patient.startTime < patient.input.arrivalTime)
        throw std::logic_error("Chua co thoi gian cho hop le.");
    return patient.startTime - patient.input.arrivalTime;
}
} // namespace clinic
