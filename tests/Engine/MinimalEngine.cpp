#include "EngineTest.h"

struct Foo {};

struct Derivate : public Foo {};

TEST(RefPtr, ImplicitUpcast) {
    OwningPtr<Derivate> owner = new Derivate();
    RefPtr<Derivate> der = owner;
    RefPtr<Foo> foo = der;
}