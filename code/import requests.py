import requests
from itertools import product

base_url = "https://www.undercity.quest/{}"

for digits in product("0123456789", repeat=4):
    code = "".join(digits)
    url = base_url.format(code)
    try:
        response = requests.get(url, timeout=5)
        print(f"{url} - Status: {response.status_code}")
    except Exception as e:
        print(f"{url} - Error: {e}")
