#pragma once
#include <string>
#include <memory>
#include <vector>
#include <utility>

class Foo {
 public:
  Foo() { }
  ~Foo() { }
  void A() { 
    printf("I'm Foo::A\n");
  }
  void B() {
    printf("I'm Foo::B\n");
  }
  static int Static();
  static std::string Geocode();
};


