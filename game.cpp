#include "game.h"

// PRIVATE

Game::GameImpl::GameImpl(const GameOptions& L, const GameOptions& R, size_t hash)
: L(L)
, R(R)
, hash(hash) {}

size_t Game::GameImpl::computeHash(const Game::GameOptions& L, const Game::GameOptions& R) {
	static const size_t	MIXER          = 0x9e3779b9;
	static const size_t DIFFERENTIATOR = 0x9e3779b97f4a7c13;

	size_t seed = 0;

	// Hash left options
	for (const auto& game : L) {
		size_t h = game.hash();
		seed ^= h + MIXER + (seed << 6) + (seed >> 2);
	}

	// Differentiate left and right options
	seed ^= DIFFERENTIATOR + (seed << 6) + (seed >> 2);

	// Hash right options
	for (const auto& game : R) {
		size_t h = game.hash();
		seed ^= game.hash() + MIXER + (seed << 6) + (seed >> 2);
	}
	return seed;
}

// PUBLIC

// Public constructor
Game::Game(std::initializer_list<Game> L, std::initializer_list<Game> R, const std::string& label)
: label_(label)
{
	GameOptions left(L.begin(), L.end()), right(R.begin(), R.end());

	Game temp = Game::create(left, right);

	impl_ = temp.impl_;
}

// Factory
// Create or retrieve a Game instance
Game Game::create(const Game::GameOptions& L, const Game::GameOptions& R, const std::string label) {
	size_t hash = GameImpl::computeHash(L, R);

	static std::unordered_map<size_t, std::weak_ptr<const GameImpl>> gameCache;
	static std::mutex cacheMutex;

	std::lock_guard<std::mutex> lock(cacheMutex);

	auto it = gameCache.find(hash);
	if (it != gameCache.end()) {
		if (auto existingImpl = it->second.lock()) {
			return Game(existingImpl, label);
		} else {
			// weak_ptr expired: remove from cache
			gameCache.erase(it);
		}
	}

	// Create new GameImpl instance
	auto newImpl = std::make_shared<const GameImpl>(L, R, hash);
	gameCache[hash] = newImpl;

	return Game(newImpl, label);
}

// Accessors

const std::string& Game::label() const { return label_; }

const Game::GameOptions& Game::L() const { return impl_->L; }

const Game::GameOptions& Game::R() const { return impl_->R; }

size_t Game::hash() const { return impl_->hash; }

// PRIVATE

// Constructor
Game::Game(std::shared_ptr<const GameImpl> impl, const std::string label)
: impl_(std::move(impl))
, label_(label) {}

struct Game::PairHash {
	std::size_t operator()(const std::pair<const Game::GameImpl*, const Game::GameImpl*>& p) const noexcept {
		std::size_t h1 = std::hash<const Game::GameImpl*>{}(p.first);
		std::size_t h2 = std::hash<const Game::GameImpl*>{}(p.second);
		return h1 ^ (h2 << 1); // Combine hashes
	}
};

struct Game::PairEqual {
	bool operator()(const std::pair<const Game::GameImpl*, const Game::GameImpl*>& lhs,
		const std::pair<const Game::GameImpl*, const Game::GameImpl*>& rhs) const {
		// Correct because of interning mechanism
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}
};

// PUBLIC

// Arithmetic

Game operator+(const Game& G, const Game& H) {
	auto key = std::make_pair(G.impl_.get(), H.impl_.get());

	// Static cache
	static std::unordered_map<std::pair<const Game::GameImpl*, const Game::GameImpl*>,
							  Game, Game::PairHash, Game::PairEqual> additionCache;
	static std::recursive_mutex cacheMutex;

	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

	// Check cache
	auto it = additionCache.find(key);
	if (it != additionCache.end())
		return it->second;

	// Not cached
	// Compute addition
	Game::GameOptions sumL, sumR;
	Game::addL(sumL, G, H);
	Game::addL(sumL, H, G);
	Game::addR(sumR, G, H);
	Game::addR(sumR, H, G);
	Game sum = Game::create(sumL, sumR);

	// Cache result
	additionCache.emplace(key, sum);

	return sum;
}

Game Game::operator-() const {
	// TODO: Memoize
	Game::GameOptions negL, negR;
	for (const Game& Gl : L())
		negL.insert(-Gl);
	for (const Game& Gr : R())
		negR.insert(-Gr);
	if (label_.empty())
		return create(negR, negL);
	else {
		// std::string newLabel = label_.front() == '-' ? label_.substr(1, label_.size() - 1) : '-' + label_;
		// return create(negR, negL, newLabel);
		return create(negR, negL, '-' + label_);
	}
}

Game operator-(const Game& G, const Game& H) { return G + (-H); }


// Comparisons

bool operator<=(const Game& G, const Game& H) {
	// TODO: Memoize
	for (const Game& Gl : G.L())
		if (H <= Gl) return false;
	for (const Game& Hr : H.R())
		if (Hr <= G) return false;
	// G <= H iff we disprove the existence statements
	return true;
}

bool operator>=(const Game& G, const Game& H) { return H <= G; }

bool operator==(const Game& G, const Game& H) { return (G <= H) && (H <= G); }

bool operator!=(const Game& G, const Game& H) { return !(G == H); }

bool operator< (const Game& G, const Game& H) { return (G <= H) && !(H <= G); }

bool operator> (const Game& G, const Game& H) { return H < G; }

bool operator||(const Game& G, const Game& H) { return !(G <= H) && !(H <= G); }


// Display

// Structural print
std::ostream& operator<<(std::ostream& out, const Game& G) {
	out << '{';
	std::string delimiter = "";
	for (const Game& Gl : G.L()) {
		out << delimiter << Gl;
		delimiter = ',';
	}
	out << '|';
	delimiter = "";
	for (const Game& Gr : G.R()) {
		out << delimiter << Gr;
		delimiter = ',';
	}
	return out << '}';
}


// PRIVATE

// Addition helpers : tail-recursiveness

void Game::addL(GameOptions& sumL, const Game& fixedGame, const Game& playedGame) {
	for (const Game& Gl : playedGame.L())
		sumL.insert(fixedGame + Gl);
}

void Game::addR(GameOptions& sumR, const Game& fixedGame, const Game& playedGame) {
	for (const Game& Gr : playedGame.R())
		sumR.insert(fixedGame + Gr);
}
