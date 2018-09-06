from enum import Enum
import gym
from gym import spaces
import random
import sys

class Colors(Enum):
    WHITE = '0'
    RED = '91'
    YELLOW = '93'
    BLUE = '94'
    GREEN = '92'
    PURPLE = '95'

class Env(gym.Env):
    def __init__(self, random_seed = 420):
        self.width = 15
        self.height = 15
        self.num_colors = len(Colors)
        self.action_space = spaces.Tuple((spaces.Discrete(self.width), spaces.Discrete(self.height)))
        self.board = [["0" for x in range(self.height)] for y in range(self.height)]
        self._randomize_board(random_seed)
        self._total_reward = 0
        self.possible_moves = self.get_possible_moves()
        random.seed()

    @property
    def total_reward(self):
        return self._total_reward

    def _randomize_board(self, random_seed):
        random.seed(random_seed)
        for x in range(self.width):
            for y in range(self.height):
                self.board[x][y] = str(random.randint(1, 5))

    def _collapse(self):
        # collapse downward
        for x in range(self.width):
            col_str = "".join(self.board[x])
            col_str = col_str.replace('0', '')
            col_str = col_str.ljust(self.width, '0')
            self.board[x] = list(col_str)
        # collapse left if column is empty
        x = self.width - 2
        while x >= -1:
            if all(elem == "0" for elem in self.board[x]):
                col = self.board.pop(x)
                self.board.append(col)
            x -= 1 

    def _is_board_empty(self):
        return all([all(elem == "0" for elem in col) for col in self.board])

    def _is_game_over(self):
        for x in range(self.width):
            for y in range(self.height):
                color = self.board[x][y]
                if color is "0":
                    break
                # check above
                if y + 1 < self.height and self.board[x][y+1] == color:
                    return False
                # check right
                if x + 1 < self.width and self.board[x+1][y] == color:
                    return False 
        return True

    def _aggregate(self, key, adj_dict):
        l = list()
        if key in adj_dict:
            for elem in adj_dict[key]:
                l.append(elem)
                l += self._aggregate(elem, adj_dict)
        return l

    def get_possible_moves(self):
        adj_dict = dict()
        for x in range(self.width):
            for y in range(self.height):
                color = self.board[x][y]
                if color is "0":
                    break
                # check above
                if y + 1 < self.height and self.board[x][y+1] == color:
                    if (x, y) not in adj_dict:
                        adj_dict[(x, y)] = list()
                    adj_dict[(x,y)].append((x, y+1)) 
                # check right
                if x + 1 < self.width and self.board[x+1][y] == color:
                    if (x, y) not in adj_dict:
                        adj_dict[(x, y)] = list()
                    adj_dict[(x,y)].append((x+1, y))
        moves = dict()
        covered = set()
        for key in adj_dict:
            if key not in covered:
                moves[key] = self._aggregate(key, adj_dict)
                for elem in moves[key]:
                    covered.add(elem)
        return moves

    def make_move(self, action):
        assert action in self.possible_moves 
        removed_tiles = [action] + self.possible_moves[action]
        for elem in removed_tiles:
            x, y = elem
            self.board[x][y] = "0"
        return len(removed_tiles)
        
    def step(self, action):
        """Take an action in the environment
        Args:
            self: this object
            action: an (x, y) integer tuple specifying the board position to play
        Returns:
            An observation object, the reward for taking the specified action,
            a boolean flag denoting whether or not the game is over, and a dict of
            diagnostic information
        """
        assert self.action_space.contains(action)
        num_removed = self.make_move(action)
        reward = (num_removed - 2)**2
        self._collapse()
        is_game_over = self._is_game_over()
        if is_game_over:
            if self._is_board_empty():
                reward += 1000
            else:
                reward -= 1000
        self.possible_moves = self.get_possible_moves()
        self._total_reward += reward
        return self.board, reward, is_game_over, {}

    def render(self, mode='terminal'):
        outfile = sys.stdout
        # top border
        outfile.write('*' * (self.width * 2 + 3) + '\n')
        for y in range(self.height - 1, -1, -1):
            outfile.write('| ')
            for x in range(0, self.width):
                color = self.board[x][y]
                if mode == 'terminal':
                    if color == '0':
                        color_code = Colors.WHITE
                    elif color == '1':
                        color_code = Colors.RED
                    elif color == '2':
                        color_code = Colors.YELLOW
                    elif color == '3':
                        color_code = Colors.GREEN
                    elif color == '4':
                        color_code = Colors.BLUE
                    elif color == '5':
                        color_code = Colors.PURPLE
                    outfile.write('\033[' + color_code.value + 'm' + color + '\033[0m ')
                else:
                    outfile.write(color + ' ')
            outfile.write('|\n')
        outfile.write('*' * (self.width * 2 + 3) + '\n')
