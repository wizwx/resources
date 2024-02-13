// #define BOOST_SML_CREATE_DEFAULT_CONSTRUCTIBLE_DEPS
#include "boost/sml.hpp"
#include <cassert>
#include <iostream>
#include <functional>

// g++ -std=c++17 order.cc -g -o ordertest && ./ordertest

namespace sml = boost::sml;

enum class ReqType : uint8_t {
  New,
  Cancel,
  Replace,
};

struct Req {
  ReqType reqtype_;
};

struct Ack {};
struct Rej {};
struct Fill {
  int qty_{};
};
struct Cancel {};
// struct Term {};

struct OrderInfo {
  int qty_{};
  int filledqty_{};
};

// status
struct Idle {};
struct Pending {};
struct PendingNew {};
struct PendingCancel {};
struct PendingReplace {};
struct New {};
struct PartialFilled {};
struct Filled {};
struct Canceled {};
struct Rejected {};


class OrderStatus {
  public:
    explicit OrderStatus() {}

    auto operator()() {
      // using namespace boost::sml;
      using namespace sml;

      const auto guardReqNos = [](const auto& event) {
        std::cout << "check req against nos" << std::endl;
        return event.reqtype_ == ReqType::New;
      };

      const auto guardReqCancel = [](const auto& event) {
        std::cout << "check req against cancel" << std::endl;
        return event.reqtype_ == ReqType::Cancel;
      };

      const auto guardReqReplace = [](const auto& event) {
        std::cout << "check req against replace" << std::endl;
        return event.reqtype_ == ReqType::Replace;
      };

      const auto guardFill = [](const auto& event, const OrderInfo& info) {
        std::cout << "check fill: qty=" << event.qty_ << " order=(" << info.qty_ << ", " << info.filledqty_ << ")" << std::endl;
        return event.qty_ + info.filledqty_ == info.qty_;
      };

      const auto guardPFill = [](const auto& event, const OrderInfo& info) {
        std::cout << "check partial fill: qty=" << event.qty_ << " order=(" << info.qty_ << ", " << info.filledqty_ << ")" << std::endl;
        return event.qty_ + info.filledqty_ < info.qty_;
      };

      const auto updateFill = [](const auto& event, OrderInfo& info) {
        info.filledqty_ += event.qty_;
        std::cout << "updateFilled called, event.qty=" << event.qty_ << " filledqty=" << info.filledqty_ << std::endl;
      };

      return make_transition_table(
          * state<Idle> + event<Req> [guardReqNos] = state<PendingNew>,
          * state<Idle> + event<Req> [guardReqCancel] = state<PendingCancel>,
          * state<Idle> + event<Req> [guardReqReplace] = state<PendingReplace>,

            state<PendingNew> + event<Rej> = state<Rejected>,
            state<PendingNew> + event<Ack> = state<New>,

            state<New> + event<Rej> = state<Rejected>,
            state<New> + event<Fill> [guardPFill] / updateFill = state<PartialFilled>,
            state<New> + event<Fill> [guardFill] / updateFill = state<Filled>,
            state<New> + event<Cancel> = state<Canceled>,

            state<PartialFilled> + event<Fill> [guardPFill] / updateFill = state<PartialFilled>,
            state<PartialFilled> + event<Fill> [guardFill] / updateFill = state<Filled>,
            state<PartialFilled> + event<Cancel> = state<Canceled>,

            // state<Filled> + event<Term> = X,
            // state<Rejected> + event<Term> = X,
            // state<Canceled> + event<Term> = X
            state<Filled>   / []{ std::cout << "order terminated since filled" << std::endl; } = X,
            state<Rejected> / []{ std::cout << "order terminated since rejected" << std::endl; } = X,
            state<Canceled> / []{ std::cout << "order terminated since canceled" << std::endl; } = X
      );
    };
};

int main() {
  OrderStatus status{};
  OrderInfo order{200, 0};
  // sml::sm<data> sm{d, Connected{1}};
  // sml::sm<data> sm{d};
  sml::sm<OrderStatus> sm{status, order};
  // std::cout << "status is " << (void*)&status << " sm is " << (void*)&sm << " size " << sizeof(sm) << " order: " << &order << std::endl;
  std::cout << "size of sml::sm<OrderStatus>: " << sizeof(sm) << " order: " << &order << std::endl;
  assert(sm.is(sml::state<Idle>));
  sm.process_event(Req{ReqType::New});
  assert(sm.is(sml::state<PendingNew>));

  // sm.process_event(Rej{});
  // assert(sm.is(sml::state<Rejected>));

  sm.process_event(Ack{});
  assert(sm.is(sml::state<New>));

  sm.process_event(Fill{100});
  assert(sm.is(sml::state<PartialFilled>));
  assert(order.filledqty_ == 100);

  sm.process_event(Fill{50});
  assert(sm.is(sml::state<PartialFilled>));
  assert(order.filledqty_ == 150);

  sm.process_event(Fill{50});
  // assert(sm.is(sml::state<Filled>));
  assert(order.filledqty_ == 200);
  assert(sm.is(sml::X));


  /*
  sm.process_event(connect{1024});
  sm.process_event(interrupt{});
  sm.process_event(connect{1025});
  sm.process_event(disconnect{});
  assert(sm.is(sml::X));
  */
}
