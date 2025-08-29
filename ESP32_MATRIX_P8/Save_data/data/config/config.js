document.getElementById("title").innerHTML = 'Espace Configuration des Scénarios'
let _type = 'null';
let _active = -1;
let scenario = [];
let fonts = []; let anims = [];

let nextBtn = document.getElementById("nextBtn");
let backBtn = document.getElementById("backBtn");
let submit = document.getElementById('submitButton');

let scripts = document.getElementById("scripts");


window.onchange = function(){
    display()
}

window.onload = function(){
    backBtn.disabled = true;
    submit.disabled = true;
    read();
    // ranger();
    
    setInterval("getState()", 1000);
}

function getState() {
    const storedObj = localStorage.getItem("connected");

    if (storedObj) {
        const parsedObj = JSON.parse(storedObj);

        if (parsedObj.expiration < new Date().getTime()) {
            localStorage.removeItem("connected");
            console.log("expired");
        }
    }
}

function display() {

    nextBtn.disabled = _active >= 1;
    backBtn.disabled = _active < 1;
    submit.disabled = _active < 1;

    // for (let i = 1; i < 11; i++) {
    for (let i = 1; i < 3; i++) {
        if (scenario[i-1] != null) {
            document.getElementById("l0"+i.toString().padStart(2, '0')).className += ' fulled';
            document.getElementById("l1"+i.toString().padStart(2, '0')).className += ' fulled';
        } else {
            document.getElementById("l0"+i.toString().padStart(2, '0')).classList.remove('fulled');
            document.getElementById("l1"+i.toString().padStart(2, '0')).classList.remove('fulled');
        }

    }
}

function getFontAndAnim(file) {
    let str = '';
    file.forEach(elm => {
        str += '<option value="'+elm.type+'">'+elm.type+'</option>'
    });
    return str;
}

function setScriptView() {

    if (_type === 'null') {
        scripts.innerHTML ='<div class="card mb-4" style="height: 680px"><h3 class="sub-title"> Choisissez une configuration ! </h3></div>'

    } else if( _type === 'Texte') {
        scripts.innerHTML = '\
        <div class="card mb-4" style="height: 690px">\
            <form id="form64">\
            <div class="card-body" style="font-size: 16px">\
                <div class="row">\
                    <div class="col-sm-3 mx-1">\
                        <p class="mb-0 mx-1">Full Text</p>\
                    </div>\
                    <div class="col-sm-8">\
                        <textarea style="border: solid black;" name="text" id="text" cols="30" rows="5" class="form-control form-control-sm"\
                        placeholder="Entrer le message a défiller." ></textarea>\
                    </div>\
                </div>\
                <hr>\
                <div class="row">\
                    <div class="col-sm-4">\
                        <div class="row mx-1">\
                            <p class="mb-0">Row alignment</p>\
                        </div>\
                        <div class="row mx-3">\
                            <select style="border: solid black;" class="select form-control form-control-sm" id="row" name="id" >\
                                <option value="top">top</option>\
                                <option value="center" selected>center</option>\
                                <option value="bottom">bottom</option>\
                            </select>\
                        </div>\
                    </div>\
                    <div class="col-sm-4">\
                        <div class="row mx-1">\
                            <p class="mb-0">Column alignment</p>\
                        </div>\
                        <div class="row mx-3">\
                            <select style="border: solid black;" class="select form-control form-control-sm" id="col" name="col" >\
                                <option value="start">start</option>\
                                <option value="center" selected>center</option>\
                                <option value="end">end</option>\
                            </select>\
                        </div>\
                    </div>\
                    <div class="col-sm-4">\
                        <div class="row mx-1">\
                            <p class="mb-0">Speed</p>\
                        </div>\
                        <div class="row mx-3">\
                            <select style="border: solid black;" class="select form-control form-control-sm" id="speed" name="speed" >\
                                <option value="1">x1</option>\
                                <option value="2">x2</option>\
                                <option value="3">x3</option>\
                                <option value="4">x4</option>\
                                <option value="5">x5</option>\
                                <option value="6">x6</option>\
                                <option value="7">x7</option>\
                                <option value="8">x8</option>\
                                <option value="9">x9</option>\
                                <option value="10">x10</option>\
                                <option value="15">x15</option>\
                                <option value="20">x20</option>\
                                <option value="25">x25</option>\
                                <option value="30">x30</option>\
                                <option value="35">x35</option>\
                                <option value="40">x40</option>\
                                <option value="45">x45</option>\
                                <option value="50">x50</option>\
                                <option value="55">x55</option>\
                                <option value="60">x60</option>\
                                <option value="65">x65</option>\
                                <option value="70">x70</option>\
                                <option value="75">x75</option>\
                                <option value="80">x80</option>\
                                <option value="85">x85</option>\
                                <option value="90">x90</option>\
                                <option value="95">x95</option>\
                                <option value="100">x100</option>\
                                <option value="120">x120</option>\
                                <option value="140">x140</option>\
                                <option value="160">x160</option>\
                                <option value="180">x180</option>\
                                <option value="200">x200</option>\
                                <option value="300">x300</option>\
                                <option value="400">x400</option>\
                                <option value="500">x500</option>\
                                <option value="600">x600</option>\
                                <option value="700">x700</option>\
                                <option value="800">x800</option>\
                                <option value="900">x900</option>\
                                <option value="1000" selected>x1000</option>\
                                <option value="1500">x1500</option>\
                                <option value="2000">x2000</option>\
                                <option value="2500">x2500</option>\
                                <option value="3000">x3000</option>\
                                <option value="3500">x3500</option>\
                                <option value="4000">x4000</option>\
                                <option value="4500">x4500</option>\
                                <option value="5000">x5000</option>\
                            </select>\
                        </div>\
                    </div>\
                </div>\
                <hr>\
                <div class= "row" >\
                    <div class="col-sm-4">\
                        <div class="row mx-1">\
                            <p class="mb-0">Text color</p>\
                        </div>\
                        <div class="row mx-3">\
                            <input type="color" name="fcolor" id="fcolor" >\
                        </div>\
                    </div>\
                    <div class="col-sm-4">\
                        <div class="row mx-1">\
                            <p class="mb-0">Background color</p>\
                        </div>\
                        <div class="row mx-3">\
                            <input type="color" name="bcolor" id="bcolor" >\
                        </div>\
                    </div>\
                    <div class="col-sm-4" >\
                        <div class="row mx-1">\
                            <p class="mb-0">Font</p>\
                        </div>\
                        <div class="row mx-3">\
                            <select style="border: solid black;" class="select form-control form-control-sm" id="ffamily" name="ffamily">'
                                +getFontAndAnim(fonts)+'\
                            </select>\
                        </div>\
                    </div>\
                </div>\
                <hr>\
                <div class="row">\
                    <div class="col-sm-4">\
                        <div class="row mx-1">\
                            <p class="mb-0">Animation</p>\
                        </div>\
                        <div class="row mx-3">\
                            <select style="border: solid black;" class="select form-control form-control-sm" id="animation" name="animation" >'
                                +getFontAndAnim(anims)+'\
                            </select>\
                        </div>\
                    </div>\
                    <div class="col-sm-4">\
                        <div class="row mx-1">\
                            <p class="mb-0">Timing</p>\
                        </div>\
                        <div class="row mx-3">\
                            <select style="border: solid black;" class="select form-control form-control-sm" id="timing" name="timing" >\
                                <option value="01">01</option>\
                                <option value="02">02</option>\
                                <option value="03">03</option>\
                                <option value="04">04</option>\
                                <option value="05">05</option>\
                                <option value="06">06</option>\
                                <option value="07">07</option>\
                                <option value="08">08</option>\
                                <option value="09">09</option>\
                                <option value="10">10</option>\
                                <option value="11">11</option>\
                                <option value="12">12</option>\
                                <option value="13">13</option>\
                                <option value="14">14</option>\
                                <option value="15">15</option>\
                                <option value="16">16</option>\
                                <option value="17">17</option>\
                                <option value="18">18</option>\
                                <option value="19">19</option>\
                                <option value="20">20</option>\
                                <option value="21">21</option>\
                                <option value="22">22</option>\
                                <option value="23">23</option>\
                                <option value="24">24</option>\
                                <option value="25">25</option>\
                                <option value="26">26</option>\
                                <option value="27">27</option>\
                                <option value="28">28</option>\
                                <option value="29">29</option>\
                                <option value="30">30</option>\
                                <option value="31">31</option>\
                                <option value="32">32</option>\
                                <option value="33">33</option>\
                                <option value="34">34</option>\
                                <option value="35">35</option>\
                                <option value="36">36</option>\
                                <option value="37">37</option>\
                                <option value="38">38</option>\
                                <option value="39">39</option>\
                                <option value="40">40</option>\
                                <option value="41">41</option>\
                                <option value="42">42</option>\
                                <option value="43">43</option>\
                                <option value="44">44</option>\
                                <option value="45">45</option>\
                                <option value="46">46</option>\
                                <option value="47">47</option>\
                                <option value="48">48</option>\
                                <option value="49">49</option>\
                                <option value="50">50</option>\
                                <option value="51">51</option>\
                                <option value="52">52</option>\
                                <option value="53">53</option>\
                                <option value="54">54</option>\
                                <option value="55">55</option>\
                                <option value="56">56</option>\
                                <option value="57">57</option>\
                                <option value="58">58</option>\
                                <option value="59">59</option>\
                                <option value="60">60</option>\
                            </select>\
                        </div>\
                    </div>\
                    <div class="col-sm-4">\
                        <div class="row mx-1">\
                            <p class="mb-0">Size</p>\
                        </div>\
                        <div class="row mx-3">\
                            <select style="border: solid black;" class="select form-control form-control-sm" id="fsize" name="fsize" >\
                                <option value="12">12</option>\
                                <option value="16">16</option>\
                                <option value="24">24</option>\
                            </select>\
                        </div>\
                    </div>\
                </div>\
                <hr class="mb-4">\
            </div>\
            </form>\
        </div>';
        getTextAreaMaxLenght();
    }
}

function getTextAreaMaxLenght() {

    let text = document.getElementById('text');
    let fsize = document.getElementById('fsize');

    fsize.addEventListener('change', function () {
        const selectedValue = +(fsize.value);
        text.maxLength = ((64/(selectedValue/2))*7);
        console.log('fsize: ', selectedValue);
        console.log('maxLength: ', text.maxLength);
    })
}

function add() {

    if(_type === 'null') {
        scenario.splice((_active), 1, null)

    } else if (_type === 'Texte') {
        let text = document.getElementById('text').value;
        let ffamily = document.getElementById('ffamily').value;
        let fsize = document.getElementById('fsize').value;
        let bcolor = document.getElementById('bcolor').value;
        let fcolor = document.getElementById('fcolor').value;
        let speed = document.getElementById('speed').value;
        let row = document.getElementById('row').value;
        let col = document.getElementById('col').value;
        let animation = document.getElementById('animation').value;
        let timing = document.getElementById('timing').value;

        if (text === '') {
            alert("Texte vide")
            e.preventDefault();

        } else {

            let script = {
                type: _type,
                text: text,
                font: ffamily,
                size: +fsize,
                bcolor: bcolor,
                fcolor: fcolor,
                animation: animation,
                speed: +speed,
                row: row,
                col: col,
                timing: +timing
            }
            scenario.splice((_active), 1, script)

        }

    }
}

function send() {

    add();

    let finalScenario = [];
    scenario.forEach( elmt => {
        if (elmt != null) {
            if(elmt.type === 'Texte'){
                console.log(elmt.type);
                elmt.fcolor = elmt.fcolor.replace('#', '');
                elmt.bcolor = elmt.bcolor.replace('#', '');
            }
            finalScenario.push(elmt);
        }
    });

    if (finalScenario.length < 1) {
        alert("Definissez au moins un scénario !");

    } else {

        const xhr = new XMLHttpRequest();
        let formData = new FormData();
        formData.append("scenario", JSON.stringify(finalScenario, null, 4));
        xhr.open("POST", "/valid", true);
        xhr.send(formData);

        alert("Scénario enregistré avec succès !");

        // Refresh the page
        
	    setTimeout(() => {
	    	location.reload();
	    }, 1000);
        
		// Refresh the page by replacing the URL with itself
		// location.replace(location.href);

    }
}

function setTypes(i) {

    switch (i) {
        case 0:
            document.getElementById('S001').value = scenario[i].type;
            break;

        case 1:
            document.getElementById('S002').value = scenario[i].type;
            break;

        default:
            console.log('No choice');
            break;
    }
}

function toggleView(i) {

    if (_active !== -1) {
       add();
    }
    
    if (scenario[i] != null) {
        toggleType(i, scenario[i].type);
    } else {
        toggleType(i, 'null');
    }

    display();

}

function toggleType(i, type) {

    _type = type;
    _active = i;
    setActive(i);
    setScriptView();
    load(i)
}

function setActive(n) {
    let tab = [
        document.getElementById("l001"),
        document.getElementById("l101"),

        document.getElementById("l002"),
        document.getElementById("l102"),

        document.getElementById("S001"),
        document.getElementById("S002")
    ];
    if ( n===0 ) {
        tab[0].className += ' active';
        tab[1].className += ' active';
        tab[4].className += ' active';

        tab[2].classList.remove('active');
        tab[3].classList.remove('active');
        tab[5].classList.remove('active');

    } else if ( n===1 ) {
        tab[2].className += ' active';
        tab[3].className += ' active';
        tab[5].className += ' active';

        tab[0].classList.remove('active');
        tab[1].classList.remove('active');
        tab[4].classList.remove('active');
    }
}

function next() {

    add();

    if (_active < 9) {

        _active = _active+1

        if (scenario[_active] === null) {
            toggleType(_active, 'null');
        } else {
            toggleType(_active, scenario[_active].type);
        }

    }
    display()

}

function back() {

    add();

    if (_active > 0) {

        _active = _active-1

        if (scenario[_active] === null) {
            toggleType(_active, 'null');
        } else {
            toggleType(_active, scenario[_active].type);
        }

    }
    display()
}

function load(i) {
    if (scenario[i] != null) {

        document.getElementById('timing').value = (scenario[i].timing).toString().padStart(2, '0');
        if (scenario[i].type === 'Texte') {

            // Définir les valeurs des champs
            document.getElementById('text').value = scenario[i].text;
            document.getElementById('ffamily').value = scenario[i].font;
            document.getElementById('fsize').value = scenario[i].size;
            document.getElementById('bcolor').value = scenario[i].bcolor;
            document.getElementById('fcolor').value = scenario[i].fcolor;
            document.getElementById('speed').value = scenario[i].speed;
            document.getElementById('row').value = scenario[i].row;
            document.getElementById('col').value = scenario[i].col;
            document.getElementById('animation').value = scenario[i].animation;

        }
    }

}

function read() {

    fetch('../jsonFiles/scenario.json')
        .then(response => response.json())
        .then(data => {
            // use the 'data' variable which contains the parsed JSON data
            scenario = data;

            let scenarioTmp = [];
            scenario.forEach(element => {
                if (element != null) {
                    if(element.type === 'Texte'){
                        element.fcolor = '#'+element.fcolor;
                        element.bcolor = '#'+element.bcolor;
                        scenarioTmp.push(element);
                    }
                }
            });
            scenario = scenarioTmp;

            if (scenario.length < 10) {
                for (let i = scenario.length; i < 10; i++) {
                    scenario.push(null);
                }
            }
            console.log("Scenario: ", scenario);

            display();

            for (let i = 0; i < scenario.length; i++) {
                if(scenario[i] != null){
                    setTypes(i);
                }
            }

            // if (scenario.length > 0){
            //     _active = 1;
            // }
        })
        .catch(error => {
            // handle any errors that occur during the fetch request
            console.error(error);
    });

    fetch('../jsonFiles/font.json')
        .then(response => response.json())
        .then(data => {
            // use the 'data' variable which contains the parsed JSON data
            fonts = data;
            console.log("Fonts: ", fonts);
        })
        .catch(error => {
            // handle any errors that occur during the fetch request
            console.error(error);
    });

    fetch('../jsonFiles/animation.json')
        .then(response => response.json())
        .then(data => {
            // use the 'data' variable which contains the parsed JSON data
            anims = data;
            console.log("Animations: ", anims);
        })
        .catch(error => {
            // handle any errors that occur during the fetch request
            console.error(error);
    });

    fetch('/admin/lighting')
        .then(response => response.text())
        .then(data => {
            let fields = data.split('|');
            console.info(fields);
            document.getElementById('lighting').value = fields[1];
            document.getElementById('lightingValue').textContent = fields[1];
            document.getElementById('auto').checked = fields[3] === '1';

        })
        .catch(error => {
            // handle any errors that occur during the fetch request
            console.error(error);
    });

}

function applyLighting() {

    const lighting = document.getElementById('lighting');
    const auto = document.getElementById('auto');

    let formData = new FormData();
        
    if(auto.checked){
        formData.append("auto", auto.checked);
    } else {
        formData.append("lighting", lighting.value);
    }
    console.log(formData);

    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/admin/lighting", true);
    xhr.send(formData);
    
    setTimeout(() => {
        alert("Luminosité enregistrée avec succès !");
    }, 1.5*1000);
}

function ranger() {
    const rangeInput = document.getElementById('lighting');
    const rangeValue = document.getElementById('lightingValue');

    rangeInput.addEventListener('change', function() {
        rangeValue.textContent = rangeInput.value;
        console.log(rangeInput.value);
        applyLighting();
    });
}

function disable() {
    
    let lighting = document.getElementById('lighting');
    let auto = document.getElementById('auto');

    console.log("disabled: ", auto.checked);

    if (auto.checked) {
        lighting.disabled = true;
        applyLighting();

    } else {
        applyLighting();
        lighting.disabled = false;
    }

}

function checkS() {
    console.log("Scenario", scenario);
    console.log("active", _active);
    console.log("len", scenario.length);

}
