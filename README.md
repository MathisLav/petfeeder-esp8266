# Petfeeder - Module embarqué
Ce git contient le code qui est dans l'ESP et qui gère les distributions de nourriture. Pour la partie interface web, voir Application Web.

# Installation
Afin d'être capable de compiler et d'uploader le programme sur un ESP, il faut d'abord installer platformio (voir [ici](https://docs.platformio.org/en/latest/core/installation.html#installation-methods)). Il faut aussi ajouter un plugin à votre IDE pour qu'il sache utiliser platformio. Nous recommandons VSCode qui est très simple d'utilisation.
Enfin, une fois le projet cloné, il suffit de l'ouvrir avec votre IDE et d'uploader à l'aide de l'extension platformio le programme sur l'ESP préalablement branché en USB à l'ordinateur.

Dès que le transfert est terminé, l'ESP génèrera automatiquement le point d'accès Wi-Fi (voir Utilisation).

# Utilisation

* Brancher l'ESP.
* Connecter un smartphone ou un ordinateur au point d'accès Wi-Fi Petfeeder.
* Charger dans un navigateur la page 192.168.4.1.
* Renseigner les identifiants d'un ou de plusieurs points d'accès Wi-Fi.
* Vous pouvez vous déconnecter du point d'accès.
* L'ESP va automatiquement récupérer la configuration au serveur cloud.

# Application web
[https://github.com/matthieu994/petfeeder-website](https://github.com/matthieu994/petfeeder-website)
