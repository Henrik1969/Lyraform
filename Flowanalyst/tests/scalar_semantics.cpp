#include <flowcontracts/scalar_semantics.hpp>
#include <cstdlib>

int main() {
    using namespace lyraform::scalar;
    auto require = [](bool condition) { if (!condition) std::abort(); };
    for (const auto source : {Type::integer, Type::boolean, Type::other, Type::outside_slice, Type::invalid})
        for (const auto target : {Type::integer, Type::boolean})
            require(compatible(source, target) == (source == target));
    require(type("int") == Type::integer && type("Bool") == Type::boolean);
    require(type("c_int") == Type::outside_slice && type("bool") == Type::outside_slice);
    require(binary("<", Type::integer, Type::integer) == Type::boolean);
    require(binary("+", Type::integer, Type::integer) == Type::integer);
    require(binary("+", Type::boolean, Type::integer) == Type::invalid);
    require(binary("+", Type::boolean, Type::boolean) == Type::invalid);
    require(unary("not", Type::boolean) == Type::boolean);
    require(unary("not", Type::integer) == Type::invalid);
    Fact fact;
    fact.statement = fact.declaration_statement = fact.expression = fact.destination = 0;
    fact.source_type = fact.destination_type = Type::integer;
    require(fact.admitted());
    fact.source_type = Type::boolean;
    require(!fact.admitted());
    fact.source_type = Type::integer;
    fact.destination = -1;
    require(!fact.admitted());
}
