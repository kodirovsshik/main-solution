import json

database_file = open("database", "r")
database = json.load(database_file)

print(database)
print(hash("oops"))
