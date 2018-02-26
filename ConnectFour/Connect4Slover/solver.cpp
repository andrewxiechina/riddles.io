#include <iostream>
#include <sys/time.h>
#include <cstring>

class Position {
  public:

    static const int WIDTH = 7;  // Height of the board
    static const int HEIGHT = 6; // Width of the board
    static const int RANGE = WIDTH * HEIGHT;
    static_assert(WIDTH < 10, "Board's width must be less than 10");
  private:
    int board[WIDTH][HEIGHT]; // 0 if cell is empty, 1 for first player and 2 for second player.
    int height[WIDTH];        // number of stones per column
    unsigned int moves;       // number of moves played since the beinning of the game.
  public:
    /**
     * Indicates whether a column is playable.
     * @param col: 0-based index of column to play
     * @return true if the column is playable, false if the column is already full.
     */
    bool canPlay(int col) const
    {
      return height[col] < HEIGHT;
    }

    /**
     * Plays a playable column.
     * This function should not be called on a non-playable column or a column making an alignment.
     *
     * @param col: 0-based index of a playable column.
     */
    void play(int col)
    {
      board[col][height[col]] = 1 + moves%2;
      height[col]++;
      moves++;
    }

    /*
     * Plays a sequence of successive played columns, mainly used to initilize a board.
     * @param seq: a sequence of digits corresponding to the 1-based index of the column played.
     *
     * @return number of played moves. Processing will stop at first invalid move that can be:
     *           - invalid character (non digit, or digit >= WIDTH)
     *           - playing a colum the is already full
     *           - playing a column that makes an aligment (we only solve non).
     *         Caller can check if the move sequence was valid by comparing the number of
     *         processed moves to the length of the sequence.
     */
    unsigned int play(std::string seq)
    {
      for(unsigned int i = 0; i < seq.size(); i++) {
        int col = seq[i] - '1';
        if(col < 0 || col >= Position::WIDTH || !canPlay(col) || isWinningMove(col)) return i; // invalid move
        play(col);
      }
      return seq.size();
    }

    void draw() const
    {
      for (int i = HEIGHT - 1; i >= 0; i--) {
          for (int j = 0; j < WIDTH; j++) {
            std::cout << board[j][i];
          }
          std::cout << std::endl;
      }
    }

    /**
     * Indicates whether the current player wins by playing a given column.
     * This function should never be called on a non-playable column.
     * @param col: 0-based index of a playable column.
     * @return true if current player makes an alignment by playing the corresponding column col.
     */
    bool isWinningMove(int col) const
    {
      int current_player = 1 + moves%2;
      // check for vertical alignments
      if(height[col] >= 3
          && board[col][height[col]-1] == current_player
          && board[col][height[col]-2] == current_player
          && board[col][height[col]-3] == current_player)
        return true;

      for(int dy = -1; dy <=1; dy++) {    // Iterate on horizontal (dy = 0) or two diagonal directions (dy = -1 or dy = 1).
        int nb = 0;                       // counter of the number of stones of current player surronding the played stone in tested direction.
        for(int dx = -1; dx <=1; dx += 2) // count continuous stones of current player on the left, then right of the played column.
          for(int x = col+dx, y = height[col]+dx*dy; x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && board[x][y] == current_player; nb++) {
            x += dx;
            y += dx*dy;
          }
        if(nb >= 3) return true; // there is an aligment if at least 3 other stones of the current user
                                 // are surronding the played stone in the tested direction.
      }
      return false;
    }

    /**
     * @return number of moves played from the beginning of the game.
     */
    unsigned int nbMoves() const
    {
      return moves;
    }

    /*
     * Default constructor, build an empty position.
     */
    Position(){
      std::memset(height, 0, sizeof(height));
      moves = 0;
      std::memset(board, 0, sizeof(board));
    }




};


class Solver {
  private:
  unsigned long long nodeCount; // counter of explored nodes.

  /**
   * Reccursively score connect 4 position using negamax variant of alpha-beta algorithm.
   * @param: alpha < beta, a score window within which we are evaluating the position.
   *
   * @return the exact score, an upper or lower bound score depending of the case:
   * - if true score of position <= alpha then true score <= return value <= alpha
   * - if true score of position >= beta then beta <= return value <= true score
   * - if alpha <= true score <= beta then return value = true score
   */
   int negamax(const Position &P, int alpha, int beta){
       nodeCount++; // increment counter of explored nodes
       int EXPLORE_SEQUENCE[7] = {3,2,4,1,5,0,6};

       if(P.nbMoves() == Position::RANGE) // check for draw game
         return 0;

       for(int x = 0; x < Position::WIDTH; x++) // check if current player can win next move
         if(P.canPlay(x) && P.isWinningMove(x))
           return (Position::RANGE+1 - P.nbMoves())/2;

       int max = -Position::RANGE/2;// lower bound for best value
       for(int x = 0; x < Position::WIDTH; x++){ // compute the score of all possible next move and keep the best one
           int i = EXPLORE_SEQUENCE[x];
           if(P.canPlay(i)) {
             Position P2(P);
             P2.play(i);               // It's opponent turn in P2 position after current player plays x column.
             int negamax_value = -negamax(P2, -beta, -alpha); // explore opponent's score within [-beta;-alpha] windows:
             max = std::max(max, negamax_value);
             alpha = std::max(alpha, negamax_value);
             if(alpha >= beta) return alpha;
           }
       }

       return max;
     }

  public:

  int solve(const Position &P, bool weak = false){
    nodeCount = 0;
    if(weak)
      return negamax(P, -1, 1);
    else
      return negamax(P, -Position::RANGE/2, Position::RANGE/2);
  }

  unsigned long long getNodeCount()
  {
    return nodeCount;
  }

};



/*
 * Get micro-second precision timestamp
 * uses unix gettimeofday function
 */
unsigned long long getTimeMicrosec() {
  timeval NOW;
  gettimeofday(&NOW, NULL);
  return NOW.tv_sec*1000000LL + NOW.tv_usec;
}


int main(int argc, char** argv) {

  Solver solver;

  bool weak = false;
  if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'w') weak = true;

  std::string line;

  for(int l = 1; std::getline(std::cin, line); l++) {
    Position P;
    if(P.play(line) != line.size())
    {
      std::cerr << "Line << " << l << ": Invalid move " << (P.nbMoves()+1) << " \"" << line << "\"" << std::endl;
    }
    else
    {
      unsigned long long start_time = getTimeMicrosec();
      int score = solver.solve(P, weak);
      unsigned long long end_time = getTimeMicrosec();

      unsigned long long count = solver.getNodeCount();
      std::cout << line << " " << score << " " << count << " " << (double)count/(end_time - start_time);
    }
    std::cout << std::endl;
  }

}
