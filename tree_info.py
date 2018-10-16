with open('tree_info', 'r') as ti:
    lines = [line[:-1] for line in ti.readlines()]

groups = list()

children = list()
terminals = list()

for line in lines:
    group, depth = line.split(';')
    if group is not '':
        group = [int(n) for n in group.split(',')]
        groups.append((group, int(depth))) 
        children.append((len(group), int(depth)))
    else:
        terminals.append(int(depth))

with open('nc', 'w') as nc:
    for n,d in children:
        nc.write(str(n) + "," + str(d) + '\n')

with open('td', 'w') as td:
    for d in terminals:
        td.write(str(d) + '\n')

with open('gr', 'w') as gr:
    for group in groups:
        gr.write(str(group[0]) + ';' + str(group[1]) + '\n')
