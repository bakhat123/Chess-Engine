import json

# get arguments
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("epd", help="input EPD file")
parser.add_argument("json", help="output JSON file")
args = parser.parse_args()

lines = []

# read EPD file
with open(args.epd, 'r') as f:
    for line in f:
        parts = line.strip().split(';')
        if len(parts) == 0:
            continue
        # create JSON object
        obj = {}
        obj['fen'] = parts[0].strip()
        for part in parts[1:]:
            part = part.strip()
            if len(part) == 0:
                continue
            key, value = part.split(maxsplit=1)
            if value.isnumeric():
                value = int(value)
            obj[key] = value
        lines.append(obj)

# write JSON
json.dump(lines, open(args.json, 'w'))
