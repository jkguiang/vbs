from itertools import product

particles = ["d", "u", "s", "c", "b", "t"]

template = "    if (pdg_id1 == {0} and pdg_id2 == {1}) {{ return {2}; }} // {3}"

combos = []
lines = []
num = 0
for pdg_id1, pdg_id2 in product([5,4,3,2,1], [5,4,3,2,1]):
    if (pdg_id1, pdg_id2) in combos or (pdg_id2, pdg_id1) in combos:
        continue
    comment = f"{particles[pdg_id1 - 1]}, {particles[pdg_id2 - 1]}"
    if pdg_id1 == pdg_id2:
        line = "    if (pdg_id1 == {0} && pdg_id2 == {1}) {{ return {2}; }} // {3}"
    else:
        line = "    if ((pdg_id1 == {0} && pdg_id2 == {1}) || (pdg_id1 == {1} && pdg_id2 == {0})) {{ return {2}; }} // {3}"
    lines.append(line.format(pdg_id1, pdg_id2, num, comment))
    combos.append((pdg_id1, pdg_id2))
    num += 1

lines.append("    else { return -1; }")
if_statements = '\n'.join(lines)

print(f"""
int getPartonPairNum(unsigned int pdg_id1, unsigned int pdg_id2)
{{
{if_statements}
}}
""")
    
# [f"{particles[pair[0]-1]}, {particles[pair[1]-1]}" for pair in list(combinations([1,2,3,4,5,6], 2))]
