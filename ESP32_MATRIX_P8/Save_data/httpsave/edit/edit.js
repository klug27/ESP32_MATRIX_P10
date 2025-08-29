document.getElementById("title").innerHTML = 'Liste des effets.'
let contents = [];
let fsInfo = {
    type: '',
    usedBytes: 0,
    totalBytes: 0,
    unsupportedFiles: false,
    isOk: false
}

window.onload = function() {
    read();
    refreshStatus();
}

function handleFileInput() {
    const fileInput = document.getElementById('upload-displayer');
    const file = fileInput.files[0];

    if (file) {
        sendGifToServer(file, file.name);
        // const reader = new FileReader();

        // reader.onload = function(event) {
        //     const gifData = event.target.result;
        //     // Envoyer le fichier GIF au serveur
        //     sendGifToServer(gifData, file.name);
        // };

        // reader.readAsDataURL(file);
    } else {
        console.log('Aucun fichier sélectionné.');
    }
}

function sendGifToServer(file, fileName) {
    const xhr = new XMLHttpRequest();
  
    xhr.onload = function() {
        // Le fichier a été complètement envoyé et la réponse du serveur a été reçue
        if (xhr.status === 200 || xhr.statusText === 'OK') {
            setTimeout(() => {
		read();
                modalUpload.click();
            }, 3*1000);
        } else {
            console.log('Une erreur s\'est produite lors de l\'envoi du fichier.');
            alert('Une erreur s\'est produite lors de l\'envoi du fichier.');
        }
    };

    let formData = new FormData();
    formData.append("data", file, '/'+fileName);

    xhr.open('POST', '/edit');
    xhr.send(formData);

    setTimeout(() => {	
	read();
	modalUpload.click();
	location.reload();
    }, 5*1000);
}

function refreshStatus() {
    // document.getElementById("status").innerHTML = "(refreshing...)";
    let xmlHttp = new XMLHttpRequest();
    xmlHttp.onload = function () {
        if (xmlHttp.status !== 200) showHttpError(xmlHttp);
        else {
            fsInfo = JSON.parse(xmlHttp.responseText);
            console.log(fsInfo);
            let status = fsInfo.type + " - ";
            if (fsInfo.isOk === 'true') {
                let percent = (1 + Math.round(99 * fsInfo.usedBytes / fsInfo.totalBytes)); // fake to see the "used" bar in any case
                let text = readableSize(fsInfo.totalBytes - fsInfo.usedBytes) + " free of " + readableSize(fsInfo.totalBytes);
                status += "<meter id='fsMeter' min='0' optimum='0' low='90' high='95' max='100' value='" + percent + "' title='" + text + "'>" + text + "</meter>"
                if (fsInfo.unsupportedFiles) {
                    status += " <span id='warning'>WARNING<span class='tooltip'>"
                        + "Filesystem&nbsp;contains&nbsp;unsupported&nbsp;filenames:<br/>"
                        + fsInfo.unsupportedFiles
                        + "</span></span>";
                }
            } else {
                status += "<span style='background-color:red;color:white;font-weight:bold'>INIT ERROR !</span>";
            }
            document.getElementById("status").innerHTML = status;
            if (fsInfo.type !== "SPIFFS") {
                // document.getElementById("mkdir").style.display = "inline";
            }
        }
    };
    xmlHttp.open("GET", "/status", true);
    xmlHttp.send(null);
}

function read() {

    // fetch('/list?dir=/Effets')
    fetch('../jsonFiles/effets.json')
        .then(response => response.json())
        .then(data => {
            // use the 'data' variable which contains the parsed JSON data
            contents = data;
            console.log("dir contents: ", contents);

            const container = document.getElementById('container');
            contents.forEach(item => {
                const listItem = document.createElement('div');
                listItem.className = 'col my-2';
                listItem.style.cursor = 'pointer';
                //
                // listItem.addEventListener('dblclick', function() {
                //     const displayer = document.getElementById('displayer');
                //     displayer.src = '/Effets/'+item.link;
                //     modal.style.display = 'block';
                // });

                const listItemCard = document.createElement('div');
                listItemCard.className = 'card';

                const listItemSVG = document.createElement('div');
                listItemSVG.className = 'col-sm-2';
                listItemSVG.innerHTML = `<svg viewBox="0 0 24 24" style="width: 50px;" fill="none" xmlns="http://www.w3.org/2000/svg"><g id="SVGRepo_bgCarrier" stroke-width="0"></g><g id="SVGRepo_tracerCarrier" stroke-linecap="round" stroke-linejoin="round"></g><g id="SVGRepo_iconCarrier"> <path fill-rule="evenodd" clip-rule="evenodd" d="M17 5H7C4.79086 5 3 6.79086 3 9V15C3 17.2091 4.79086 19 7 19H17C19.2091 19 21 17.2091 21 15V9C21 6.79086 19.2091 5 17 5Z" stroke="#000000" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"></path> <path d="M9.42932 9.83956C9.82451 9.96364 10.2455 9.74387 10.3696 9.34868C10.4936 8.95349 10.2739 8.53253 9.87868 8.40845L9.42932 9.83956ZM8.842 9L8.84112 8.25C8.83449 8.25001 8.82786 8.25011 8.82123 8.25029L8.842 9ZM6 12L6.74974 12.0198C6.75009 12.0066 6.75009 11.9934 6.74974 11.9802L6 12ZM8.842 15L8.82123 15.7497C8.82786 15.7499 8.83449 15.75 8.84112 15.75L8.842 15ZM9.654 14.876L9.87868 15.5916C9.88376 15.59 9.88881 15.5883 9.89385 15.5866L9.654 14.876ZM11.684 12L12.4339 11.988C12.4274 11.5785 12.0935 11.25 11.684 11.25V12ZM8.842 11.25C8.42779 11.25 8.092 11.5858 8.092 12C8.092 12.4142 8.42779 12.75 8.842 12.75V11.25ZM12.829 15C12.829 15.4142 13.1648 15.75 13.579 15.75C13.9932 15.75 14.329 15.4142 14.329 15H12.829ZM14.329 9C14.329 8.58579 13.9932 8.25 13.579 8.25C13.1648 8.25 12.829 8.58579 12.829 9H14.329ZM15.987 15C15.987 15.4142 16.3228 15.75 16.737 15.75C17.1512 15.75 17.487 15.4142 17.487 15H15.987ZM17.487 11.667C17.487 11.2528 17.1512 10.917 16.737 10.917C16.3228 10.917 15.987 11.2528 15.987 11.667H17.487ZM15.987 11.667C15.987 12.0812 16.3228 12.417 16.737 12.417C17.1512 12.417 17.487 12.0812 17.487 11.667H15.987ZM16.737 10.333H17.487C17.487 10.3266 17.4869 10.3201 17.4868 10.3137L16.737 10.333ZM18.0211 9.74971C18.4351 9.73805 18.7614 9.39295 18.7497 8.9789C18.7381 8.56486 18.393 8.23865 17.9789 8.2503L18.0211 9.74971ZM16.737 12.417C17.1512 12.417 17.487 12.0812 17.487 11.667C17.487 11.2528 17.1512 10.917 16.737 10.917V12.417ZM15.474 10.917C15.0598 10.917 14.724 11.2528 14.724 11.667C14.724 12.0812 15.0598 12.417 15.474 12.417V10.917ZM16.737 10.917C16.3228 10.917 15.987 11.2528 15.987 11.667C15.987 12.0812 16.3228 12.417 16.737 12.417V10.917ZM18 12.417C18.4142 12.417 18.75 12.0812 18.75 11.667C18.75 11.2528 18.4142 10.917 18 10.917V12.417ZM9.87868 8.40845C9.54293 8.30302 9.19303 8.24959 8.84112 8.25L8.84288 9.75C9.04178 9.74977 9.23955 9.77997 9.42932 9.83956L9.87868 8.40845ZM8.82123 8.25029C6.79481 8.30643 5.19679 9.99329 5.25026 12.0198L6.74974 11.9802C6.7181 10.7811 7.66369 9.78294 8.86277 9.74971L8.82123 8.25029ZM5.25026 11.9802C5.19679 14.0067 6.79481 15.6936 8.82123 15.7497L8.86277 14.2503C7.66369 14.2171 6.7181 13.2189 6.74974 12.0198L5.25026 11.9802ZM8.84112 15.75C9.19303 15.7504 9.54293 15.697 9.87868 15.5916L9.42932 14.1604C9.23955 14.22 9.04178 14.2502 8.84288 14.25L8.84112 15.75ZM9.89385 15.5866C11.433 15.0671 12.4599 13.6123 12.4339 11.988L10.9341 12.012C10.9496 12.984 10.3352 13.8545 9.41415 14.1654L9.89385 15.5866ZM11.684 11.25H8.842V12.75H11.684V11.25ZM14.329 15V9H12.829V15H14.329ZM17.487 15V11.667H15.987V15H17.487ZM17.487 11.667V10.333H15.987V11.667H17.487ZM17.4868 10.3137C17.4789 10.0105 17.718 9.75824 18.0211 9.74971L17.9789 8.2503C16.849 8.28209 15.9581 9.22241 15.9872 10.3523L17.4868 10.3137ZM16.737 10.917H15.474V12.417H16.737V10.917ZM16.737 12.417H18V10.917H16.737V12.417Z" fill="#000000"></path> </g></svg>`

                const listItemTextContent = document.createElement('div');
                listItemTextContent.className = 'col-sm-8 my-auto';
                listItemTextContent.textContent = item.link;
                listItemTextContent.style.fontSize = "large";

                const listItemSVGDelete = document.createElement('div');
                listItemSVGDelete.className = 'col-sm-2 my-auto';
                listItemSVGDelete.innerHTML = `<svg fill="#ff0000" viewBox="0 0 32 32" style="width: 25px;" version="1.1" xmlns="http://www.w3.org/2000/svg"><g id="SVGRepo_bgCarrier" stroke-width="0"></g><g id="SVGRepo_tracerCarrier" stroke-linecap="round" stroke-linejoin="round"></g><g id="SVGRepo_iconCarrier"> <title>cross-round</title> <path d="M0 16q0 3.264 1.28 6.208t3.392 5.12 5.12 3.424 6.208 1.248 6.208-1.248 5.12-3.424 3.392-5.12 1.28-6.208-1.28-6.208-3.392-5.12-5.088-3.392-6.24-1.28q-3.264 0-6.208 1.28t-5.12 3.392-3.392 5.12-1.28 6.208zM4 16q0-3.264 1.6-6.016t4.384-4.352 6.016-1.632 6.016 1.632 4.384 4.352 1.6 6.016-1.6 6.048-4.384 4.352-6.016 1.6-6.016-1.6-4.384-4.352-1.6-6.048zM9.76 20.256q0 0.832 0.576 1.408t1.44 0.608 1.408-0.608l2.816-2.816 2.816 2.816q0.576 0.608 1.408 0.608t1.44-0.608 0.576-1.408-0.576-1.408l-2.848-2.848 2.848-2.816q0.576-0.576 0.576-1.408t-0.576-1.408-1.44-0.608-1.408 0.608l-2.816 2.816-2.816-2.816q-0.576-0.608-1.408-0.608t-1.44 0.608-0.576 1.408 0.576 1.408l2.848 2.816-2.848 2.848q-0.576 0.576-0.576 1.408z"></path> </g></svg>`;
                
                const row = document.createElement('div');
                row.className = 'row';

                const listItemCardBody = document.createElement('div');
                listItemCardBody.className = 'card-body';

                listItemSVG.addEventListener('click', function() {
                    const displayer = document.getElementById('displayer');
                    displayer.src = '/Effets/'+item.link;
                    modal.style.display = 'block';
                });

                listItemTextContent.addEventListener('click', function() {
                    const displayer = document.getElementById('displayer');
                    displayer.src = '/Effets/'+item.link;
                    modal.style.display = 'block';
                });
                
                listItemSVGDelete.onclick = function(ev) {
                    if (confirm(`Voulez-vous vraiment supprimer ?`)) httpDelete('/Effets/'+item.link);
                    else console.info('Cancelled...');
                    ev.preventDefault();
                };

                row.appendChild(listItemSVG);
                row.appendChild(listItemTextContent);
                row.appendChild(listItemSVGDelete);
                listItemCardBody.appendChild(row)
                listItemCard.appendChild(listItemCardBody);
                listItem.appendChild(listItemCard);
                container.appendChild(listItem);
            });
        })
        .catch(error => {
            // handle any errors that occur during the fetch request
            console.error(error);
        });
}

function httpDelete(path) {
    const xmlHttp = new XMLHttpRequest();
    xmlHttp.onload = function() {
        // Le fichier a été complètement envoyé et la réponse du serveur a été reçue
        if (xmlHttp.status === 200 || xmlHttp.statusText === 'OK') {
            location.reload();
        } else {
            console.log('Une erreur s\'est produite lors de l\'envoi du fichier.');
            alert('Une erreur s\'est produite lors de l\'envoi du fichier.');
        }
    };
    let formData = new FormData();
    formData.append("path", path);
    console.log(formData)
    xmlHttp.open("DELETE", "/edit");
    xmlHttp.send(formData);
}

function showHttpError(request) {
    alert("ERROR: [" + request.status + "] " + request.responseText);
}

function readableSize(bytes) {
    if (bytes < 1024) return bytes + " B";
    let units = [' KiB', ' MiB', ' GiB', ' TiB', 'PiB'];
    let i = -1;
    do {
        bytes = bytes / 1024;
        i++;
    } while (bytes > 1024);
    return bytes.toFixed(2) + units[i];
}

// Récupérez la référence de la fenêtre modale
const modal = document.getElementById('modal');

// Ajoutez un gestionnaire d'événement pour fermer la fenêtre modale lorsque l'utilisateur clique en dehors de celle-ci
modal.addEventListener('click', function(event) {
    if (event.target === modal) {
        modal.style.display = 'none';
    }
});

const modalUpload = document.getElementById('modal-upload');

const uploadButton = document.getElementById('upload-button');

// Ajoutez un gestionnaire d'événement pour ouvrir la fenêtre modale lorsque le bouton est cliqué
uploadButton.addEventListener('click', function() {
    modalUpload.style.display = 'block';
});

modalUpload.addEventListener('click', function(event) {
    if (event.target === modalUpload) {
        modalUpload.style.display = 'none';
    }
});
