import sys
from bs4 import BeautifulSoup

html = "./dataset/raw/mocap.html"

def find_all(filename, key):
    f = open(filename, "r")

    data = f.read()

    soup = BeautifulSoup(data, "html.parser")

    trs = soup.select("tr")

    subject_num = "-1"
    sequnce_num = "-1"
    res = []

    for tr in trs:
        # print("\n-------------\n")
        # print(tr)
        # print("\n--------\n")
        if tr.text.find("Subject #") >= 0:
            subject_num = tr.text[tr.text.find("Subject #") + 9:tr.text.find("(")].strip()
            if len(subject_num) == 1:
                subject_num = "0" + subject_num
            # print("subject: " + subject_num + "\n")
        if tr.text.find("Feedback") >= 0 and (tr.text.find("120") >= 0 or tr.text.find("60") >= 0) and tr.text.find(key) >= 0:
            sequnce_num = tr.select("td")[1].text
            if len(sequnce_num) == 1:
                sequnce_num = "0" + sequnce_num
            # print("sequnce: " + sequnce_num + "\n")
            res.append([subject_num + "-" + sequnce_num, tr.select("td")[2].text, tr.select("td")[8].text])

        # print("\n-------------\n")

    f.close()
    return res

if __name__ == "__main__":
    print(find_all(html, sys.argv[len(sys.argv) - 1]))