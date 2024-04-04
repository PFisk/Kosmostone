import json

f = open("./data/fireballs.json")

data = json.load(f)

for i in data["fireballs"]:
    print(i["reportAmount"])

f.close()