FROM gcc:latest
WORKDIR /usr/local/app

#download and build Toy
RUN git clone --depth 1 --branch v2-beta-2 https://gitea.krgamestudios.com/krgamestudios/Toy.git
RUN cd Toy && make release

#start with the newly built repl
CMD ["Toy/out/repl"]

#how to build and run:
# sudo docker build -t <image_name> .
# sudo docker run -it <image_name>
# echo <code> | docker run -i <image_name>
