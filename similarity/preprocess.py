input = open("./dataset/GIT_zizi.json", "r")
output = open("./dataset/simple.json", "w+")

flag = False

while True:
    ch = input.read(1)
    if ch == "":
        break

    output.write(ch)

    # if ch == ".":
    #     digits = input.read(2)
    #     output.write(digits)
    #     while True:
    #         digit = input.read(1)
    #         if digit < "0" or digit > "9":
    #             output.write(digit)
    #             break
    if ch == "," and flag:
        output.write("\n")
    flag = ch == "]"

input.close()
output.close()
