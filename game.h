#pragma once

#include <set>
#include <memory>
#include <string>
#include <ostream>
#include <unordered_map>
#include <mutex>

// TODO : factory method
// TODO : Return pointers instead of Game objects
class Game
{
public:
	using GameOptions = std::set<Game>;

	// Public constructor
	// Calls the factory method
	Game(std::initializer_list<Game> L, std::initializer_list<Game> R, const std::string& label = "");
	
	// Accessors
	const std::string& label() const;
	const GameOptions& L() const;
	const GameOptions& R() const;
	size_t hash() const;

private:
	// Factory
	// Create or retrieve a Game instance
	static Game create(const GameOptions& L, const GameOptions& R, const std::string label = "");

	// Internal implementation class
	class GameImpl {
	public:
		GameOptions L;
		GameOptions R;
		size_t hash;

		GameImpl(const GameOptions& L, const GameOptions& R, size_t hash);

		static size_t computeHash(const GameOptions& L, const GameOptions& R);
	};

	// Constructor
	Game(std::shared_ptr<const GameImpl> impl, const std::string label);

	// Binary operators hash
	struct PairHash;
	struct PairEqual;

	// Unary hash
	struct Hash;
	struct Equal;

public:
	// Arithmetic
	
	// G + H := {G+HL,GL+H|G+HR,GR+H}
	friend Game operator+(const Game&, const Game&);
	// -G := {-GR|-GL}
		   Game operator-() const;
	// G - H := G + (-H)
	friend Game operator-(const Game&, const Game&);
	

	// Comparisons
	
	// [G <= H] := ![H <= exists GL] && ![exists HR <= G]
	friend bool operator<=(const Game&, const Game&);
	// [G >= H] := [H <= G]
	friend bool operator>=(const Game&, const Game&);
	// [G == H] := [G <= H] && [H <= G]
	friend bool operator==(const Game&, const Game&);
	// [G != H] := ![G == H]
	friend bool operator!=(const Game&, const Game&);
	// [G < H] := [G <= H] && ![H <= G]
	friend bool operator<(const Game&, const Game&);
	// [G > H] := [H < G]
	friend bool operator>(const Game&, const Game&);
	// [G || H] := ![G <= H] && ![H <= G]
	friend bool operator||(const Game&, const Game&);


	// Display

	// cout << G;
	// > {GL|GR};
	friend std::ostream& operator<<(std::ostream&, const Game&);

private:
	// Additions helpers : tail-recursiveness

	static void addL(GameOptions& sumL, const Game& fixedGame, const Game& playedGame);
	static void addR(GameOptions& sumR, const Game& fixedGame, const Game& playedGame);

	// Member attributes

	std::shared_ptr<const GameImpl> impl_;
	std::string label_;
};
