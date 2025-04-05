wsl -d Ubuntu-22.04
sudo apt update
sudo apt install -y docker.io
sudo systemctl enable docker
sudo service docker start

docker --version
docker run hello-world

wsl -d Ubuntu-22.04 -u root
adduser szczkr
usermod -aG sudo szczkr
wsl -d Ubuntu-22.04 --set-default-user szczkr
wsl -d Ubuntu-22.04
sudo whoami

