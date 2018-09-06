import math
import random

class Node():
    def __init__(self, action, env):
        self._parent = None
        self._children = list()
        self._action = action
        self._env = env
        self._q = 0
        self._n = 0

    @property
    def state(self):
        return self._state

    @property 
    def q(self):
        return self._q

    @q.setter
    def q(self, value):
        self._q = value

    @property
    def n(self):
        return self._n

    @n.setter
    def n(self, value):
        self._n = value

    @property
    def children(self):
        return self._children

    @property
    def reward(self)
        return self._env.total_reward

    def expand(self):
        if self._has_children():
            print('Tried to expand a node which was already expanded')
            return False
        actions = list(self._env.get_possible_moves().keys())
        for action in actions:
            env = copy.deepcopy(self._env)
            env.step(action)
            self._children.append(env)
        return len(action) > 0

    def has_children(self):
        return len(self._children)

    def reset(self):
        self._children.clear()
        self._q = 0
        self._n = 0

class MCTS():
    def __init__(self, env, priors=None):
        if random_seed is not None:
            random.seed(random_seed)
        else:
            random.seed()
        self._root = Node(None, env) 
        self._curr = self._root
        self._ranked_list = None

    def reset(self):
        self._root.reset()
        self._curr = self._root

    def tree_policy(self, node): 
        choice = node
        while choice.has_children():
            choice = best_child(choice)
        return choice

    def default_policy(self, node):
        cur = node
        while cur.expand():
            cur = random.choice(cur.children)
        cur.children.clear()
        return cur.reward

    def best_child(self, node): 
        N = node.n
        children = node.children
        max_score = float('-inf') 
        best = list()
        for child in children:
            n = child.n
            q = child.q
            UCB1 = q/n + math.sqrt(2) * math.sqrt(math.log(N) / n) 
            if UCB1 > max_score:
                max_score = UCB1
                best = [child]
            elif UCB1 == max_score:
                best.append(child)
        return random.choice(best)

    def backprop(self, node, q):
        while node.parent is not None:
            node.q += q
            node.n += 1
            node = node.parent

    @property
    def ranked_list(self):
        return self._ranked_list


