import requests

r = requests.get('https://raw.githubusercontent.com/PFisk/Kosmostone/main/data/fireballs.json', data = {'key':'value'}, auth=('user', 'passwd'))
r.text      # response as a string
r.content   # response as a byte string
            #     gzip and deflate transfer-encodings automatically decoded 
data = r.json()    # return python object from json! this is what you probably want!

for i in data["fireballs"]:
    print(i["reportAmount"])