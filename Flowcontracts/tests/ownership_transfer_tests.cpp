#include <flowcontracts/ownership_transfer.hpp>
#include <iostream>
#include <stdexcept>

int main() {
    using namespace flowcontracts;
    try {
        // Carrier labels are deliberately unrelated to the Text runtime.
        for (const auto type : {"OwnedBuffer", "OwnedSocket", "Outcome<Packet,IoFailure>"}) {
            std::vector<OwnershipFunction> functions{{10,0,type,false,0},{20,1,"int",true,0}};
            std::vector<OwnershipOperation> operations{
                {0,0,0,10,100,-1,"producer",{}},
                {1,1,0,10,-1,-1,"return_value",{100}},
                {2,2,1,20,200,10,"call",{}}
            };
            OwnershipTransfer transfer{0,1,2,100,200,10,20,type,"completion:0"};
            auto require=[](bool condition) { if(!condition) throw std::runtime_error("uniform transfer regression"); };
            require(ownership_transfer_refusal(transfer,operations,functions).empty());
            const auto roundtrip=read_ownership_transfer(ownership_transfer_fact(transfer),"$.transfer");
            require(ownership_transfer_refusal(roundtrip,operations,functions).empty());
            auto stale=operations;
            stale.push_back({3,3,0,10,-1,-1,"use",{100}});
            require(!ownership_transfer_refusal(transfer,stale,functions).empty());
            auto duplicate=operations;
            duplicate.push_back({3,3,1,20,201,10,"call",{}});
            require(!ownership_transfer_refusal(transfer,duplicate,functions).empty());
            auto overwritten=operations;
            overwritten.push_back({3,3,1,20,200,-1,"assignment",{}});
            require(!ownership_transfer_refusal(transfer,overwritten,functions).empty());
            auto wrong_type=transfer; wrong_type.value_type="Other";
            require(!ownership_transfer_refusal(wrong_type,operations,functions).empty());
            auto wrong_owner=transfer; wrong_owner.destination_owner=100;
            require(!ownership_transfer_refusal(wrong_owner,operations,functions).empty());
        }
        std::cout << "Uniform ownership transfer: 3 carrier-independent cases and refusal checks PASS\n";
    } catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
