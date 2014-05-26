function to_carrier(country, value) {
    var subtext = "";
    switch(country)
    {
    case "234":
        switch(value) {
        case "000": subtext = "BT"; break;
        case "010": subtext = "O2"; break;
        case "015": subtext = "Vodafone"; break;
        }
        break;
    case "302":
        switch(value) {
        case "220": subtext = "Telus"; break;
        case "610": subtext = "Virgin Mobile"; break;
        case "720": subtext = "Rogers"; break;
        }
        break;
    case "311":
        switch(value) {
        case "480": subtext = "Verizon Wireless"; break;
        }
        break;
    case "374":
        switch(value) {
        case "130": subtext = "Digicel"; break;
        }
        break;
    case "505":
        switch(value) {
        case "001": subtext = "Telstra"; break;
        case "002": subtext = "Optus"; break;
        case "003": subtext = "Vodafone"; break;
        }
        break;
    case "510":
        switch(value) {
        case "001": subtext = "Indosat"; break;
        case "010": subtext = "Telkomsel"; break;
        case "010": subtext = "XL"; break;
        }
        break;

    default:
        subtext = "";
        break;
    }

    return subtext;
}

function to_country(value) {
    var subtext = "";
    switch(value)
    {
    case "001": subtext = "Test (Worldwide)"; break;
    case "412": subtext = "Afghanistan"; break;
    case "276": subtext = "Albania"; break;
    case "603": subtext = "Algeria"; break;
    case "544": subtext = "American Samoa (USA)"; break;
    case "213": subtext = "Andorra"; break;
    case "631": subtext = "Angola"; break;
    case "365": subtext = "Anguilla (UK)"; break;
    case "344": subtext = "Antigua & Barbuda"; break;
    case "722": subtext = "Argentina"; break;
    case "283": subtext = "Armenia"; break;
    case "363": subtext = "Aruba (Netherlands)"; break;
    case "505": subtext = "Australia"; break;
    case "232": subtext = "Austria"; break;
    case "400": subtext = "Azerbaijan"; break;
    case "364": subtext = "Bahamas"; break;
    case "426": subtext = "Bahrain"; break;
    case "470": subtext = "Bangladesh"; break;
    case "342": subtext = "Barbados"; break;
    case "257": subtext = "Belarus"; break;
    case "206": subtext = "Belgium"; break;
    case "702": subtext = "Belize"; break;
    case "616": subtext = "Benin"; break;
    case "350": subtext = "Bermuda"; break;
    case "402": subtext = "Bhutan"; break;
    case "736": subtext = "Bolivia"; break;
    case "218": subtext = "Bosnia & Herzegovina"; break;
    case "652": subtext = "Botswana"; break;
    case "724": subtext = "Brazil"; break;
    case "348": subtext = "Virgin Islands (UK)"; break;
    case "528": subtext = "Brunei"; break;
    case "284": subtext = "Bulgaria"; break;
    case "613": subtext = "Burkina Faso"; break;
    case "642": subtext = "Burundi"; break;
    case "456": subtext = "Cambodia"; break;
    case "624": subtext = "Cameroon"; break;
    case "302": subtext = "Canada"; break;
    case "625": subtext = "Cape Verde"; break;
    case "346": subtext = "Cayman Islands] (UK)"; break;
    case "623": subtext = "Central African Republic"; break;
    case "622": subtext = "Chad"; break;
    case "730": subtext = "Chile"; break;
    case "460": subtext = "China"; break;
    case "732": subtext = "Columbia"; break;
    case "654": subtext = "Comoros"; break;
    case "629": subtext = "Congo"; break;
    case "548": subtext = "Cook Islands (NZ)"; break;
    case "712": subtext = "Costa Rica"; break;
    case "219": subtext = "Croatia"; break;
    case "368": subtext = "Cuba"; break;
    case "280": subtext = "Cyprus"; break;
    case "230": subtext = "Czech Republic"; break;
    case "630": subtext = "Democratic Congo"; break;
    case "238": subtext = "Denmark"; break;
    case "638": subtext = "Djibouti"; break;
    case "366": subtext = "Dominica"; break;
    case "370": subtext = "Dominican Republic"; break;
    case "514": subtext = "East Timor"; break;
    case "740": subtext = "Ecuador"; break;
    case "602": subtext = "Egypt"; break;
    case "706": subtext = "El Salvador"; break;
    case "627": subtext = "Equatorial Guinea"; break;
    case "657": subtext = "Eritrea"; break;
    case "248": subtext = "Estonia"; break;
    case "636": subtext = "Ethiopia"; break;
    case "288": subtext = "Faroe Islands (Denmark)"; break;
    case "542": subtext = "Fiji"; break;
    case "244": subtext = "Finland"; break;
    case "208": subtext = "France"; break;
    case "547": subtext = "French Polynesia"; break;
    case "628": subtext = "Gabon"; break;
    case "607": subtext = "Gambia"; break;
    case "282": subtext = "Georgia"; break;
    case "262": subtext = "Germany"; break;
    case "620": subtext = "Ghana"; break;
    case "266": subtext = "Gibraltar (UK)"; break;
    case "202": subtext = "Greece"; break;
    case "290": subtext = "Greenland (Denmark)"; break;
    case "352": subtext = "Grenada"; break;
    case "340": subtext = "Guadeloupe (France)"; break;
    case "704": subtext = "Guatemala"; break;
    case "611": subtext = "Guinea"; break;
    case "632": subtext = "Guinea-Bissau"; break;
    case "738": subtext = "Guyana"; break;
    case "372": subtext = "Haiti"; break;
    case "708": subtext = "Honduras"; break;
    case "454": subtext = "Hong Kong"; break;
    case "216": subtext = "Hungary"; break;
    case "274": subtext = "Iceland"; break;
    case "404": case "405": subtext = "India"; break;
    case "510": subtext = "Indonesia"; break;
    case "432": subtext = "Iran"; break;
    case "418": subtext = "Iraq"; break;
    case "272": subtext = "Ireland"; break;
    case "222": subtext = "Italy"; break;
    case "425": subtext = "Israel"; break;
    case "612": subtext = "Ivory Coast"; break;
    case "338": subtext = "Jamaica"; break;
    case "440": subtext = "Japan"; break;
    case "416": subtext = "Jordan"; break;
    case "401": subtext = "Kazakhstan"; break;
    case "639": subtext = "Kenya"; break;
    case "545": subtext = "Kiribati"; break;
    case "467": subtext = "North Korea"; break;
    case "450": subtext = "South Korea"; break;
    case "212": subtext = "Kosovo"; break;
    case "419": subtext = "Kuwait"; break;
    case "437": subtext = "Krygyzstan"; break;
    case "457": subtext = "Laos"; break;
    case "247": subtext = "Latvia"; break;
    case "415": subtext = "Lebanon"; break;
    case "651": subtext = "Lesotho"; break;
    case "618": subtext = "Liberia"; break;
    case "606": subtext = "Libya"; break;
    case "295": subtext = "Liechenstein"; break;
    case "246": subtext = "Lithuania"; break;
    case "270": subtext = "Luxembourg"; break;
    case "455": subtext = "Macau"; break;
    case "294": subtext = "Macedonia"; break;
    case "646": subtext = "Madagascar"; break;
    case "650": subtext = "Malawi"; break;
    case "502": subtext = "Malaysia"; break;
    case "472": subtext = "Maldives"; break;
    case "610": subtext = "Mali"; break;
    case "278": subtext = "Malta"; break;
    case "340": subtext = "Martinique (France)"; break;
    case "609": subtext = "Mauritania"; break;
    case "617": subtext = "Mauritius"; break;
    case "334": subtext = "Mexico"; break;
    case "550": subtext = "Micronesia"; break;
    case "259": subtext = "Moldova"; break;
    case "212": subtext = "Monaco"; break;
    case "428": subtext = "Mongolia"; break;
    case "297": subtext = "Montenegro"; break;
    case "354": subtext = "Montserrat (UK)"; break;
    case "604": subtext = "Morocco"; break;
    case "643": subtext = "Mozambique"; break;
    case "414": subtext = "Myanmar"; break;
    case "649": subtext = "Namibia"; break;
    case "536": subtext = "Nauru"; break;
    case "429": subtext = "Nepal"; break;
    case "204": case "362": subtext = "Netherlands"; break;
    case "546": subtext = "New Caledonia (France)"; break;
    case "530": subtext = "New Zealand"; break;
    case "710": subtext = "Nicaragua"; break;
    case "614": subtext = "Niger"; break;
    case "621": subtext = "Nigeria"; break;
    case "555": subtext = "Niue"; break;
    case "242": subtext = "Norway"; break;
    case "422": subtext = "Oman"; break;
    case "410": subtext = "Pakistan"; break;
    case "552": subtext = "Palau"; break;
    case "714": subtext = "Panama"; break;
    case "537": subtext = "Papua New Guinea"; break;
    case "744": subtext = "Paraguay"; break;
    case "716": subtext = "Peru"; break;
    case "515": subtext = "Philippines"; break;
    case "260": subtext = "Poland"; break;
    case "268": subtext = "Portugal"; break;
    case "330": subtext = "Puerto Rico"; break;
    case "427": subtext = "Qatar"; break;
    case "647": subtext = "Reunion (France)"; break;
    case "226": subtext = "Romania"; break;
    case "250": subtext = "Russia"; break;
    case "635": subtext = "Rwanda"; break;
    case "356": subtext = "Saint Kitts & Nevis"; break;
    case "358": subtext = "Saint Lucia"; break;
    case "308": subtext = "Saint Pierre & Miquelon (France)"; break;
    case "360": subtext = "Saint Vincent & Grenadines"; break;
    case "549": subtext = "Samoa"; break;
    case "292": subtext = "San Marino"; break;
    case "626": subtext = "Sao Tome & Principe"; break;
    case "420": subtext = "Saudi Arabia"; break;
    case "608": subtext = "Senegal"; break;
    case "220": subtext = "Serbia"; break;
    case "633": subtext = "Seychelles"; break;
    case "619": subtext = "Sierra Leone"; break;
    case "525": subtext = "Singapore"; break;
    case "231": subtext = "Slovakia"; break;
    case "293": subtext = "Slovenia"; break;
    case "540": subtext = "Solomon Islands"; break;
    case "637": subtext = "Somalia"; break;
    case "655": subtext = "South Africa"; break;
    case "659": subtext = "South Sudan"; break;
    case "214": subtext = "Spain"; break;
    case "413": subtext = "Sri Lanka"; break;
    case "634": subtext = "Sudan"; break;
    case "746": subtext = "Suriname"; break;
    case "653": subtext = "Swaziland"; break;
    case "240": subtext = "Sweden"; break;
    case "228": subtext = "Switzerland"; break;
    case "417": subtext = "Syria"; break;
    case "466": subtext = "Taiwan"; break;
    case "436": subtext = "Tajikistan"; break;
    case "640": subtext = "Tanzania"; break;
    case "520": subtext = "Thailand"; break;
    case "615": subtext = "Togo"; break;
    case "539": subtext = "Tonga"; break;
    case "374": subtext = "Trinidad & Tobago"; break;
    case "605": subtext = "Tunisia"; break;
    case "286": subtext = "Turkey"; break;
    case "438": subtext = "Turkmenistan"; break;
    case "376": subtext = "Turks & Caicos Islands"; break;
    case "553": subtext = "Tuvalu"; break;
    case "641": subtext = "Uganda"; break;
    case "255": subtext = "Ukraine"; break;
    case "424": subtext = "United Arab Emirates"; break;
    case "234": case "235": subtext = "United Kingdom"; break;
    case "310": case "311": subtext = "United States of America"; break;
    case "748": subtext = "Uruguay"; break;
    case "434": subtext = "Uzbekistan"; break;
    case "541": subtext = "Vanuatu"; break;
    case "734": subtext = "Venezuela"; break;
    case "452": subtext = "Vietnam"; break;
    case "421": subtext = "Yemen"; break;
    case "645": subtext = "Zambia"; break;
    case "648": subtext = "Zimbabwe"; break;
    case "901": subtext = "Global / Satellite"; break;
    default:
        subtext = "";
        break;
    }
    return subtext;
}
