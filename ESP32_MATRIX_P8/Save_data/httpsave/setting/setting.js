
let _visibility = 0;
let _other = [0, 0, 0];
let _mode = '';
let _disabled = true;

let settings = document.getElementById('settings');

window.onload = function() {
    setSettingView();
    setActive(_visibility);

    setInterval("getState()", 500);
}

// Wi-Fi Config functions
function toggleVisibility(i){
    _visibility = i;
    setActive(i);
    setSettingView();
}

// Global functions
function setActive(n) {
    let tab = [
        document.getElementById("l01"),
        document.getElementById("l02"),
    ]
    if ( n===1 ) {
        tab[0].className += ' active';
        tab[1].classList.remove('active');
    } else if ( n===2 ) {
        tab[0].classList.remove('active');
        tab[1].className += ' active';
    }
}

function setSettingView() {

    if (_visibility === 1) {
        settings.innerHTML = '<iframe src="./sub/general.html" style="width: 100%; height: 570px; border-radius: 2%"></iframe>';

    } else if (_visibility === 2) {
        settings.innerHTML = '<iframe src="./sub/lighting.html" style="width: 100%; height: 570px; border-radius: 2%"></iframe>';
    }
}

