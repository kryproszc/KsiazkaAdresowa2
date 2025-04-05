wsl -d Ubuntu-22.04
sudo apt update
sudo apt install -y docker.io
sudo systemctl enable docker
sudo service docker start

docker --version
docker run hello-world
