// TEST DOUBLES ONLY. Never link this file into the application.
// Provides A/B/C interfaces to exercise Leader before those modules are delivered.
#include "Simulation.h"
#include <limits>
#include <stdexcept>

namespace clinic {
bool failNextEnqueueForLeaderTest = false;
bool failNextStatisticsResetForLeaderTest = false;
PatientStore::PatientStore() : head_(nullptr), size_(0) {}
PatientStore::~PatientStore() { clear(); }
bool PatientStore::add(const PatientInput& input, std::string& error) {
    if (findById(input.id) || containsTicket(input.ticketNumber)) {
        error = "Duplicate";
        return false;
    }
    Node* node = new Node{Patient{input}, nullptr};
    Node** link = &head_;
    while (*link && ((*link)->patient.input.arrivalTime < input.arrivalTime ||
           ((*link)->patient.input.arrivalTime == input.arrivalTime &&
            (*link)->patient.input.ticketNumber < input.ticketNumber))) link = &(*link)->next;
    node->next = *link;
    *link = node;
    ++size_;
    error.clear();
    return true;
}
const Patient* PatientStore::findById(const std::string& id) const {
    for (const Node* n = head_; n; n = n->next) if (n->patient.input.id == id) return &n->patient;
    return nullptr;
}
bool PatientStore::containsTicket(int ticket) const {
    for (const Node* n = head_; n; n = n->next) if (n->patient.input.ticketNumber == ticket) return true;
    return false;
}
int PatientStore::nextTicketNumber() const {
    int result = 0;
    for (const Node* n = head_; n; n = n->next)
        if (n->patient.input.ticketNumber > result) result = n->patient.input.ticketNumber;
    if (result == std::numeric_limits<int>::max()) throw std::overflow_error("Ticket overflow");
    return result + 1;
}
int PatientStore::size() const { return size_; }
bool PatientStore::isEmpty() const { return size_ == 0; }
const PatientStore::Node* PatientStore::head() const { return head_; }
void PatientStore::resetStates() { for (Node* n = head_; n; n = n->next) resetPatientState(n->patient); }
void PatientStore::clear() { while (head_) { Node* n = head_; head_ = n->next; delete n; } size_ = 0; }

LinkedQueue::LinkedQueue() : front_(nullptr), rear_(nullptr), size_(0) {}
LinkedQueue::~LinkedQueue() { clear(); }
void LinkedQueue::enqueue(Patient* patient) {
    Node* node = new Node{patient, nullptr};
    if (rear_) rear_->next = node; else front_ = node;
    rear_ = node;
    ++size_;
}
Patient* LinkedQueue::dequeue() {
    if (!front_) return nullptr;
    Node* node = front_;
    Patient* p = node->patient;
    front_ = node->next;
    if (!front_) rear_ = nullptr;
    delete node;
    --size_;
    return p;
}
int LinkedQueue::size() const { return size_; }
void LinkedQueue::clear() { while (front_) dequeue(); }

// Deliberately a linear ordered-array substitute, not B's heap implementation.
PriorityHeap::PriorityHeap() : items_(nullptr), size_(0), capacity_(0) {}
PriorityHeap::~PriorityHeap() { delete[] items_; }
bool PriorityHeap::comesBefore(const Patient* a, const Patient* b) {
    if (a->input.priority != b->input.priority) return a->input.priority < b->input.priority;
    if (a->input.arrivalTime != b->input.arrivalTime) return a->input.arrivalTime < b->input.arrivalTime;
    return a->input.ticketNumber < b->input.ticketNumber;
}
void PriorityHeap::push(Patient* p) {
    if (size_ == capacity_) {
        const int capacity = capacity_ + 8;
        Patient** next = new Patient*[capacity];
        for (int i = 0; i < size_; ++i) next[i] = items_[i];
        delete[] items_;
        items_ = next;
        capacity_ = capacity;
    }
    int i = size_;
    while (i > 0 && comesBefore(p, items_[i-1])) { items_[i] = items_[i-1]; --i; }
    items_[i] = p;
    ++size_;
}
Patient* PriorityHeap::pop() {
    if (!size_) return nullptr;
    Patient* p = items_[0];
    for (int i = 1; i < size_; ++i) items_[i-1] = items_[i];
    --size_;
    return p;
}
int PriorityHeap::size() const { return size_; }
void PriorityHeap::clear() { size_ = 0; }

ClinicRoom::ClinicRoom() : roomId_(-1), specialtyId_(-1), current_(nullptr) {}
void ClinicRoom::configure(int id, int specialty) { roomId_ = id; specialtyId_ = specialty; }
void ClinicRoom::enqueue(Patient* p, bool priority) {
    if (failNextEnqueueForLeaderTest) { failNextEnqueueForLeaderTest = false; throw std::bad_alloc(); }
    if (priority && p->input.priority != Priority::Normal) priority_.push(p); else normal_.enqueue(p);
}
Patient* ClinicRoom::startNext(int minute, bool priority) {
    if (current_) return nullptr;
    current_ = priority && priority_.size() ? priority_.pop() : normal_.dequeue();
    if (current_) {
        current_->status = PatientStatus::InService;
        current_->assignedRoomId = roomId_;
        current_->startTime = minute;
        current_->finishTime = minute + current_->input.serviceDuration;
    }
    return current_;
}
Patient* ClinicRoom::finishIfDue(int minute) {
    if (!current_ || minute < current_->finishTime) return nullptr;
    Patient* p = current_;
    p->status = PatientStatus::Completed;
    current_ = nullptr;
    return p;
}
void ClinicRoom::reset() { normal_.clear(); priority_.clear(); current_ = nullptr; }
int ClinicRoom::id() const { return roomId_; }
int ClinicRoom::specialtyId() const { return specialtyId_; }
bool ClinicRoom::isBusy() const { return current_ != nullptr; }
int ClinicRoom::normalQueueSize() const { return normal_.size(); }
int ClinicRoom::priorityQueueSize() const { return priority_.size(); }
int ClinicRoom::waitingCount() const { return normal_.size() + priority_.size(); }
const Patient* ClinicRoom::currentPatient() const { return current_; }

Statistics::Statistics() : rooms_(nullptr), roomCount_(0), maxSystemWaiting_(0), completedCount_(0) {}
Statistics::~Statistics() { delete[] rooms_; }
void Statistics::reset(int count) {
    if (failNextStatisticsResetForLeaderTest) {
        failNextStatisticsResetForLeaderTest = false;
        throw std::bad_alloc();
    }
    RoomStats* next = new RoomStats[count];
    for (int i = 0; i < count; ++i) next[i].roomId = i+1;
    delete[] rooms_;
    rooms_ = next;
    roomCount_ = count;
    for (auto& p : priority_) p = PriorityStats{};
    maxSystemWaiting_ = completedCount_ = 0;
}
void Statistics::recordStart(const Patient& p) {
    auto& s = priority_[static_cast<int>(p.input.priority)];
    ++s.startedCount;
    s.totalWait += waitingTime(p);
    if (waitingTime(p) > s.maxWait) s.maxWait = waitingTime(p);
}
void Statistics::recordCompletion() { ++completedCount_; }
void Statistics::observeQueues(int index, int waiting, int total) {
    if (waiting > rooms_[index].maxWaiting) rooms_[index].maxWaiting = waiting;
    if (total > maxSystemWaiting_) maxSystemWaiting_ = total;
}
const PriorityStats& Statistics::byPriority(Priority p) const { return priority_[static_cast<int>(p)]; }
const RoomStats& Statistics::byRoom(int i) const { return rooms_[i]; }
int Statistics::maxSystemWaiting() const { return maxSystemWaiting_; }
int Statistics::completedCount() const { return completedCount_; }
} // namespace clinic
