import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import networkx as nx

# 准备图和布局
G1 = nx.Graph()
G1.add_edges_from([('A', 'B'), ('B', 'C'), ('C', 'A')])
pos1 = {'A': (0.3, 0.5), 'B': (0.1, 0.1), 'C': (0.5, 0.1)}


G2 = nx.complete_graph(4)
mapping2 = {i: ch for i, ch in enumerate(['A', 'B', 'C', 'D'])}
G2 = nx.relabel_nodes(G2, mapping2)
pos2 = {
    'A': (0.1, 0.5),
    'B': (0.1, 0.1),
    'C': (0.5, 0.5),
    'D': (0.5, 0.1)  
}

G3 = nx.Graph()
G3.add_edges_from([('A', 'B'), ('B', 'D'), ('D', 'C'), ('C', 'A')])
pos3 = {'A': (0, 1), 'B': (0, 0), 'D': (1, 0), 'C': (1, 1)}

G4 = nx.Graph()
G4.add_edges_from([('A', 'B'), ('A', 'C'), ('B', 'C'), ('B', 'D'), ('C', 'E'), ('D', 'E')])
pos4 = {'A': (0.5, 2), 'B': (0, 1), 'C': (1, 1), 'D': (0, 0), 'E': (1, 0)}

G5 = nx.Graph()
G5.add_edges_from([
    ('A', 'B'), ('A', 'C'),
    ('B', 'C'), ('B', 'D'), ('B', 'E'),
    ('C', 'E'), ('C', 'F'), ('D', 'E'), ('E', 'F')
])
pos5 = {
    'A': (0.5, 0.5), 'B': (0.3, 0.3), 'C': (0.7, 0.3),
    'D': (0.1, 0.1), 'E': (0.5, 0.1), 'F': (0.9, 0.1)
}

G6 = nx.Graph()
G6.add_edges_from([
    ('A', 'B'), ('A', 'C'), ('A', 'D'),
    ('B', 'C'), ('B', 'D')
])
pos6 = {
    'A': (0, 1),  # 顶部
    'B': (0, 0),    # 左下
    'C': (1, 1),    # 右下
    'D': (1, 0)     # 底部
}


G7 = nx.Graph()
G7.add_edges_from([
    ('A', 'B'), ('A', 'C'), ('A', 'D'),('A','E'),
    ('B', 'C'), ('B', 'D'),
    ('C', 'D')
])
pos7 = {
    'A': (0, 1),  # 顶部
    'B': (0, 0),    # 左下
    'C': (1, 1),    # 右下
    'D': (1, 0),    # 底部
    'E': (0.5, 1.5)
}

G8 = nx.Graph()
G8.add_edges_from([
    ('A', 'B'), ('A', 'C'), ('A', 'D'), ('A', 'E'),
    ('B', 'C'), ('B', 'D'), ('B', 'E'),
    ('C', 'D'), ('C', 'E'),
    ('D', 'E')
])

pos8 = {
    'A': (0.5, 2),   # 顶部
    'B': (0, 1.2),   # 左上
    'C': (1, 1.2),   # 右上
    'D': (0.2, 0),   # 左下
    'E': (0.8, 0)    # 右下
}


titles = [
    "Clique3 (C3)",
    "Clique4 (C4)",
    "Rectangle (R4)",
    "House (H5)",
    "Tri_Tri (T6)",
    "HOOF4(H4)",
    "TELE5(T5)",
    "Clique5(C5)",
]

graphs = [G1, G2, G3, G4, G5 ,G6 ,G7 ,G8]
positions = [pos1, pos2, pos3, pos4, pos5 ,pos6, pos7, pos8]

# 创建大图，2行3列布局
fig, axes = plt.subplots(2, 4, figsize=(12, 6), constrained_layout=True)
axes = axes.flatten()

for i, ax in enumerate(axes):
    if i < len(graphs):
        nx.draw(graphs[i], pos=positions[i], ax=ax,
                with_labels=True,
                node_color='white', edgecolors='black',
                node_size=1000, font_size=12, font_weight='bold')
        ax.set_title(titles[i], fontsize=14)
        ax.axis('off')
    else:
        ax.axis('off')  # 空白子图隐藏坐标轴

plt.savefig("motifs_all_in_one.png", dpi=300)
plt.close()
