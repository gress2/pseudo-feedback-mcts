import copy
import math
import random

class Node():
    def __init__(self, action, env):
        self._parent = None
        self._children = list()
        self._action = action
        self._terminal = False
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
    def parent(self):
        return self._parent

    @property
    def children(self):
        return self._children

    @property
    def reward(self):
        return self._env.total_reward
    
    @property
    def terminal(self):
        return self._terminal

    def expand(self):
        if self.has_children():
            print('Tried to expand a node which was already expanded')
            return False
        actions = list(self._env.get_possible_moves().keys())
        if len(actions) == 0:
            self._terminal = True
            return False
        for action in actions:
            env = copy.deepcopy(self._env)
            env.step(action)
            child = Node(action, env)
            child._parent = self 
            self._children.append(child)
        return True

    def has_children(self):
        return len(self._children)

    def reset(self):
        self._children.clear()
        self._terminal = False
        self._q = 0
        self._n = 0

class MCTS():
    def __init__(self, env, random_seed=None, priors=None):
        if random_seed is not None:
            random.seed(random_seed)
        else:
            random.seed()
        self._root = Node(None, env) 
        self._root.n = 1
        self._curr = self._root
        self._ranked_list = None

    def reset(self):
        self._root.reset()
        self._curr = self._root

    def tree_policy(self, node): 
        choice = node
        while choice.has_children():
            choice = self.best_child(choice)
        if choice.expand():
            return self.best_child(choice)
        return choice

    def default_policy(self, node):
        cur = node
        while not cur.terminal and cur.expand():
            cur = random.choice(cur.children)
        reward = cur.reward
        print('found playout with reward: ' + str(reward))
        node.children.clear()
        return reward

    def best_child(self, node): 
        N = node.n
        children = node.children
        max_score = float('-inf') 
        best = list()
        avg_q = sum(child.q for child in children) / len(children)
        for child in children:
            n = child.n
            q = child.q
            if n == 0:
                UCB1 = float('inf')
            else:
                UCB1 = q/n + avg_q * math.sqrt(math.log(N) / n) 
            if UCB1 > max_score:
                max_score = UCB1
                best = [child]
            elif UCB1 == max_score:
                best.append(child)
        return random.choice(best)

    def search(self, iterations):
        while not self._curr.terminal:
            self._curr._env.render()
            for _ in range(iterations):
                leaf = self.tree_policy(self._curr)
                reward = self.default_policy(leaf)
                self.backprop(leaf, reward)
            self._curr = self.best_child(self._curr)
        return self._curr.reward

    def backprop(self, node, q):
        while node.parent is not None:
            node.q += q
            node.n += 1
            node = node.parent

    @property
    def ranked_list(self):
        return self._ranked_list


