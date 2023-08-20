import random
cards = []

with open("cards.data", "r") as data:
    data = data.readlines()
    for line in data:
        line = line[:-1].split('"')
        cards.append((line[1], line[2].split(" ")[1:]))

with open("enemyCards.data", "w") as player:
    for i in range(5):
        nums = input("Enemy Values (Top, Right, bottom, left) seberated by space:\n")
        nums = nums.split(" ")

        for element in cards:
            card = element[1]
            if card[0] == nums[0]:
                if card[1] == nums[2]:
                    if card[2] == nums[1]:
                        if card[3] == nums[3]:
                              player.write(f"{element[0]}\n")  
                              print(f"Cardname: {element[0]}\n")
        #choice = random.choice(cards)
        #cards.remove(choice)
        #print(choice)
        #player.write(choice)
        #player.write("\n")