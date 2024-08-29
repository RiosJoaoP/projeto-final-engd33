from random import random
from math import pi

tamanho = 100


def generate_random_array(titulo, max):
    txt = f"float {titulo} [{tamanho}] = " + "{\n"
    for _ in range(tamanho):
        value = random()*max
        txt += f"\t{value}"
        if (_ < tamanho-1):
            txt += ",\n"
    txt += "\n};\n\n"
    return txt


arrays = [
    {
        "title": "vetorCorrenteX",
        "max": 4
    },
    {
        "title": "vetorCorrenteY",
        "max": 4
    },
    {
        "title": "vetorCorrenteZ",
        "max": 4
    },
    {
        "title": "vetorVelAngX",
        "max": 10*pi
    },
    {
        "title": "vetorVelAngY",
        "max": 10*pi
    },
    {
        "title": "vetorVelAngZ",
        "max": 10*pi
    },
    {
        "title": "vetorPosicaoX",
        "max": 1000
    },
    {
        "title": "vetorPosicaoY",
        "max": 1000
    },
    {
        "title": "vetorPosicaoZ",
        "max": 1000
    },
]

with open("../Core/Src/lookuptable.c", 'w') as f:
    for array in arrays:
        f.writelines(generate_random_array(array["title"], array["max"]))
