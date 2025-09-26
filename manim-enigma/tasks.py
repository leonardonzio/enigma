from invoke.tasks import task
from invoke import run

@task
def compile(c):
    run("gcc -fPIC -shared -o ../enigma.so ../enigma.c")

@task(pre=[compile])
def manim(c):
    run("manim -pql main.py")
