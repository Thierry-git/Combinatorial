#include "game.h"
#include <iostream>
#include <cassert>

using namespace std;

int main() {
    // Base games
    Game zero({}, {}, "0");
    Game one({ zero }, {}, "1");
    Game minusOne({}, { zero }, "-1");

    // Verify basic games
    std::cout << "Zero: " << zero << std::endl;
    std::cout << "One: " << one << std::endl;
    std::cout << "Minus One: " << minusOne << std::endl;

    // Verify that 0 == 0
    assert(zero == zero);
    
    // Verify that 1 != -1
    assert(one != minusOne); // TODO: debug this

    // Create star: {0|0}
    Game star({ zero }, { zero }, "*");
    std::cout << "Star (*): " << star << std::endl;

    // Negation tests
    Game negOne = -one;
    std::cout << "Negative One (-1): " << negOne << std::endl;
    assert(negOne == minusOne);

    Game negMinusOne = -minusOne;
    std::cout << "Negative Minus One (--1): " << negMinusOne << std::endl;
    assert(negMinusOne == one);

    Game negStar = -star;
    std::cout << "Negative Star (-*): " << negStar << std::endl;
    assert(negStar == star); // Star is its own negative

    // Addition tests
    Game onePlusMinusOne = one + minusOne;
    std::cout << "1 + (-1): " << onePlusMinusOne << std::endl;
    assert(onePlusMinusOne == zero);

    Game starPlusStar = star + star;
    std::cout << "* + *: " << starPlusStar << std::endl;
    assert(starPlusStar == zero); // * + * is equivalent to 0

    Game onePlusStar = one + star;
    std::cout << "1 + *: " << onePlusStar << std::endl;

    // Comparison tests
    assert(one > zero);
    assert(minusOne < zero);
    assert(star || zero); // Star is fuzzy with zero

    // More complex games
    Game up({ zero }, {}, "Up");
    Game down({}, { zero }, "Down");
    Game fuzzy({ up }, { down }, "Fuzzy");

    std::cout << "Up: " << up << std::endl;
    std::cout << "Down: " << down << std::endl;
    std::cout << "Fuzzy: " << fuzzy << std::endl;

    // Comparisons involving Up and Down
    assert(up > zero);
    assert(down < zero);
    assert(fuzzy || zero); // Fuzzy is incomparable with zero

    // Test addition involving complex games
    Game upPlusDown = up + down;
    std::cout << "Up + Down: " << upPlusDown << std::endl;
    assert(upPlusDown == zero);

    // Test double negation
    Game negNegOne = -(-one);
    assert(negNegOne == one);

    // Testing that addition is commutative for some games
    Game a = one + star;
    Game b = star + one;
    assert(a == b);

    // Testing that addition is associative
    Game c = (one + star) + minusOne;
    Game d = one + (star + minusOne);
    assert(c == d);

    // Printing out the results
    std::cout << "All tests passed successfully!" << std::endl;

    return 0;
}
