
#include "test.h"

TEST_CASE("Input Expression", "[ParseKeySequence]") {
  // Empty
  CHECK(parse_input("") == (KeySequence{ }));

  // A has to be pressed.
  // "A"  =>  +A ~A
  CHECK(parse_input("A") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));

  // A has to be pressed first then B. A can still be hold.
  // "A B"  =>  +A ~A +B ~B
  CHECK(parse_input("A B") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::UpAsync),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::UpAsync),
  }));

  // A has to be pressed first then B. A must not be released in between.
  // "A{B}"  =>  +A +B ~B ~A
  CHECK(parse_input("A{B}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::UpAsync),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));
  CHECK_THROWS(parse_input("{B}"));

  // A has to be pressed first then B, then C. None must be released in between.
  // "A{B{C}}"  =>  +A +B +C ~C ~A ~B
  CHECK(parse_input("A{B{C}}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::UpAsync),
    KeyEvent(Key::B, KeyState::UpAsync),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));

  // A and B have to be pressed together, order does not matter.
  // "(A B)"  =>  *A *B +A +B
  CHECK(parse_input("(A B)") == (KeySequence{
    KeyEvent(Key::A, KeyState::DownAsync),
    KeyEvent(Key::B, KeyState::DownAsync),
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
  }));

  // A has to be pressed first then B and C together. A can be released any time.
  // "A(B C)"  =>  +A ~A *B *C +B +C
  CHECK(parse_input("A(B C)") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::UpAsync),
    KeyEvent(Key::B, KeyState::DownAsync),
    KeyEvent(Key::C, KeyState::DownAsync),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
  }));

  // A has to be pressed first then B, then C. A has to be released last.
  // "A{B C}"  =>  +A +B ~B +C ~A
  CHECK(parse_input("A{B C}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::UpAsync),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::UpAsync),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));

  // A has to be pressed first then B and C together. A has to be released last.
  // "A{(B C)}"  =>  +A *B *C +B +C ~A ~B ~C
  CHECK(parse_input("A{(B C)}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::DownAsync),
    KeyEvent(Key::C, KeyState::DownAsync),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::UpAsync),
    KeyEvent(Key::B, KeyState::UpAsync),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));

  // A and B have to be pressed together, order does not matter. Then C, then D.
  // "(A B){C D}"  =>  *A *B +A +B +C ~C +D ~D ~A ~B
  CHECK(parse_input("(A B){C D}") == (KeySequence{
    KeyEvent(Key::A, KeyState::DownAsync),
    KeyEvent(Key::B, KeyState::DownAsync),
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::UpAsync),
    KeyEvent(Key::D, KeyState::Down),
    KeyEvent(Key::D, KeyState::UpAsync),
    KeyEvent(Key::B, KeyState::UpAsync),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));

  // Not
  CHECK(parse_input("!A") == (KeySequence{
    KeyEvent(Key::A, KeyState::Not),
  }));
  CHECK(parse_input("A !A B") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::UpAsync),
    KeyEvent(Key::A, KeyState::Not),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::UpAsync),
  }));
  CHECK_THROWS(parse_input("!"));
  CHECK_THROWS(parse_input("!(A B)"));
  CHECK_THROWS(parse_input("!A{B}"));
  CHECK_THROWS(parse_input("A{!B}"));

  // Output on release
  CHECK_THROWS(parse_input("A ^ B"));

  // Timeout
  CHECK(parse_input("A 1000ms") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    make_timeout_ms(1000),
  }));

  CHECK(parse_input("A{1000ms}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    make_timeout_ms(1000),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));

  CHECK(parse_input("A 1000ms B") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    make_timeout_ms(1000),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::UpAsync),
  }));

  CHECK(parse_input("A{1000ms B}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    make_timeout_ms(1000),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::UpAsync),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));

  CHECK_NOTHROW(parse_input("A 10000000ms"));
  CHECK_THROWS(parse_input("1000ms A"));
  CHECK_THROWS(parse_input("(A 1000ms)"));
  CHECK_THROWS(parse_input("(A !1000ms)"));

  // Not Timeout
  CHECK(parse_input("A !1000ms") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    make_not_timeout_ms(1000),
  }));

  CHECK(parse_input("A{!1000ms}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    make_not_timeout_ms(1000),
    KeyEvent(Key::A, KeyState::Up), // Not Async!
  }));

  CHECK(parse_input("A{1000ms !1000ms}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    make_timeout_ms(1000),
    make_not_timeout_ms(1000),
    KeyEvent(Key::A, KeyState::Up), // Not Async!
  }));

  // Timeouts are merged to minimize undefined behaviour
  CHECK(parse_input("A{1000ms 100ms !1000ms !100ms !10ms 1000ms}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    make_timeout_ms(1100),
    make_not_timeout_ms(1110),
    make_timeout_ms(1000),
    KeyEvent(Key::A, KeyState::UpAsync),
  }));
}

//--------------------------------------------------------------------

TEST_CASE("Output Expression", "[ParseKeySequence]") {
  // Empty
  CHECK(parse_output("") == (KeySequence{ }));

  // Press A.
  // "A"  =>  +A
  CHECK(parse_output("A") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
  }));

  // Press A and then B.
  // "A B"  =>  +A -A +B -B
  CHECK(parse_output("A B") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::Up),
  }));

  // Press A and keep hold while pressing B.
  //   "A{B}"  =>  +A +B -B -A
  CHECK(parse_output("A{B}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::Up),
    KeyEvent(Key::A, KeyState::Up),
  }));
  CHECK_THROWS(parse_output("{B}"));

  // Press A and B together, order does not matter.
  //   "(A B)"  =>  +A +B
  CHECK(parse_output("(A B)") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
  }));

  // Press A, B, C together, order does not matter.
  //   "(A B C)"  =>  +A +B +C
  CHECK(parse_output("(A B C)") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
  }));

  // Press A first and then B and C, order does not matter.
  // "A(B C)"  =>  +A -A +B +C -C -B
  CHECK(parse_output("A(B C)") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::Up),
    KeyEvent(Key::B, KeyState::Up),
  }));

  // Press A and keep hold while pressing B and then C.
  // "A{B C}"  =>  +A +B B- +C -C -A
  CHECK(parse_output("A{B C}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::Up),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::Up),
    KeyEvent(Key::A, KeyState::Up),
  }));

  // Press A and keep hold while pressing B and C, order does not matter.
  // "A{(B C)}"  =>  +A +B +C -C -B -A
  CHECK(parse_output("A{(B C)}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::Up),
    KeyEvent(Key::B, KeyState::Up),
    KeyEvent(Key::A, KeyState::Up),
  }));

  // Press A and B together, order does not matter,
  // keep hold while pressing C and then D.
  // "(A B){C D}"  =>  +A +B +C -C +D -D -B -A
  CHECK(parse_output("(A B){C D}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::Up),
    KeyEvent(Key::D, KeyState::Down),
    KeyEvent(Key::D, KeyState::Up),
    KeyEvent(Key::B, KeyState::Up),
    KeyEvent(Key::A, KeyState::Up),
  }));

  // Press A, B and C together.
  // "A{B{C}}"  =>  +A +B +C -C -B -A
  CHECK(parse_output("A{B{C}}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::C, KeyState::Down),
    KeyEvent(Key::C, KeyState::Up),
    KeyEvent(Key::B, KeyState::Up),
    KeyEvent(Key::A, KeyState::Up),
  }));

  // Not
  CHECK(parse_output("!A") == (KeySequence{
    KeyEvent(Key::A, KeyState::Not),
  }));

  // Output on release
  CHECK(parse_output("A ^ B") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    KeyEvent(Key::none, KeyState::OutputOnRelease),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::Up),
  }));
  CHECK(parse_output("^ A B") == (KeySequence{
    KeyEvent(Key::none, KeyState::OutputOnRelease),
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::Up),
  }));
  CHECK(parse_output("A B ^") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::Up),
    KeyEvent(Key::none, KeyState::OutputOnRelease),
  }));
  CHECK(parse_output("^") == (KeySequence{
    KeyEvent(Key::none, KeyState::OutputOnRelease),
  }));
  CHECK_THROWS(parse_output("A ^ B ^ C"));
  CHECK_THROWS(parse_output("^ A ^ B"));
  CHECK_THROWS(parse_output("(A ^ B)"));
  CHECK_THROWS(parse_output("A{^ B}"));
  CHECK_THROWS(parse_output("A^{B}"));

  // Virtual
  CHECK(parse_output("Virtual0") == (KeySequence{
    KeyEvent(Key::first_virtual, KeyState::Down),
  }));
  CHECK_NOTHROW(parse_output("Virtual100") == (KeySequence{
    KeyEvent(static_cast<Key>(*Key::first_virtual + 100), KeyState::Down),
  }));
  CHECK_THROWS(parse_output("Virtual1000"));

  // Timeout
  CHECK(parse_output("A 1000ms") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    make_output_timeout_ms(1000),
  }));

  CHECK(parse_output("A{1000ms}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    make_output_timeout_ms(1000),
    KeyEvent(Key::A, KeyState::Up),
  }));

  CHECK(parse_output("A 1000ms B") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::A, KeyState::Up),
    make_output_timeout_ms(1000),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::Up),
  }));

  CHECK(parse_output("A{1000ms B}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    make_output_timeout_ms(1000),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::B, KeyState::Up),
    KeyEvent(Key::A, KeyState::Up),
  }));

  CHECK(parse_output("1000ms A") == (KeySequence{
    make_output_timeout_ms(1000),
    KeyEvent(Key::A, KeyState::Down),
  }));

  CHECK(parse_output("(A B){1000ms}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    make_output_timeout_ms(1000),
    KeyEvent(Key::B, KeyState::Up),
    KeyEvent(Key::A, KeyState::Up),
  }));

  CHECK_NOTHROW(parse_output("A 10000000ms"));

  // Not Timeouts are not allowed
  CHECK_THROWS(parse_output("(A !1000ms)"));
  CHECK_THROWS(parse_output("A !1000ms"));
  CHECK_THROWS(parse_output("A{!1000ms}"));
  CHECK_THROWS(parse_output("A{1000ms !1000ms}"));

  // Timeouts are merged to minimize undefined behaviour
  CHECK(parse_output("A{1000ms 100ms 1000ms}") == (KeySequence{
    KeyEvent(Key::A, KeyState::Down),
    make_output_timeout_ms(2100),
    KeyEvent(Key::A, KeyState::Up),
  }));
}

//--------------------------------------------------------------------

TEST_CASE("Key names", "[ParseKeySequence]") {
  CHECK(parse_output("(Digit1 2 KeyA B Any)") == (KeySequence{
    KeyEvent(Key::Digit1, KeyState::Down),
    KeyEvent(Key::Digit2, KeyState::Down),
    KeyEvent(Key::A, KeyState::Down),
    KeyEvent(Key::B, KeyState::Down),
    KeyEvent(Key::any, KeyState::Down),
  }));

  CHECK(parse_output("(OSLeft MetaLeft OSRight MetaRight)") == (KeySequence{
    KeyEvent(Key::MetaLeft, KeyState::Down),
    KeyEvent(Key::MetaLeft, KeyState::Down),
    KeyEvent(Key::MetaRight, KeyState::Down),
    KeyEvent(Key::MetaRight, KeyState::Down),
  }));

  CHECK(parse_output("(Virtual0 Virtual255)") == (KeySequence{
    KeyEvent(Key::first_virtual, KeyState::Down),
    KeyEvent(static_cast<Key>(static_cast<int>(Key::first_virtual) + 255),
      KeyState::Down),
  }));
  CHECK_THROWS(parse_output("Virtual256"));
}

//--------------------------------------------------------------------

TEST_CASE("Key code", "[ParseKeySequence]") {
  CHECK(parse_output("1ms (0 9 F 01 61439 0x1 0XefFF)") == (KeySequence{
    make_output_timeout_ms(1),
    KeyEvent(Key::Digit0, KeyState::Down),
    KeyEvent(Key::Digit9, KeyState::Down),
    KeyEvent(Key::F, KeyState::Down),
    KeyEvent(static_cast<Key>(1), KeyState::Down),
    KeyEvent(static_cast<Key>(61439), KeyState::Down),
    KeyEvent(static_cast<Key>(0x1), KeyState::Down),
    KeyEvent(static_cast<Key>(0xEFFF), KeyState::Down),
  }));

  CHECK_THROWS(parse_output("00"));
  CHECK_THROWS(parse_output("0F"));
  CHECK_THROWS(parse_output("-01"));
  CHECK_THROWS(parse_output("61440"));
  CHECK_THROWS(parse_output("0xF000"));
  CHECK_THROWS(parse_output("01z"));
  CHECK_THROWS(parse_output("0xEFz"));
  CHECK_THROWS(parse_output("0x1ms"));
  CHECK_THROWS(parse_output("0xEFms"));
}
