var imageTag = false;
var theSelection = false;

// Check for Browser & Platform for PC & IE specific bits
// More details from: http://www.mozilla.org/docs/web-developer/sniffer/browser_type.html
var clientPC = navigator.userAgent.toLowerCase(); // Get client info
var clientVer = parseInt(navigator.appVersion); // Get browser version

var is_ie = ((clientPC.indexOf("msie") != -1) && (clientPC.indexOf("opera") == -1));
var is_nav = ((clientPC.indexOf('mozilla')!=-1) && (clientPC.indexOf('spoofer')==-1)
                && (clientPC.indexOf('compatible') == -1) && (clientPC.indexOf('opera')==-1)
                && (clientPC.indexOf('webtv')==-1) && (clientPC.indexOf('hotjava')==-1));
var is_moz = 0;

var is_win = ((clientPC.indexOf("win")!=-1) || (clientPC.indexOf("16bit") != -1));
var is_mac = (clientPC.indexOf("mac")!=-1);


code = new Array();
tags = new Array('[b]','[/b]','[i]','[/i]','[u]','[/u]','[q]','[/q]','[pic]','[/pic]','[url=',']ссылка[/url]');
imageTag = false;



// Replacement for arrayname.length property
function getarraysize(thearray) {
	for (i = 0; i < thearray.length; i++) {
		if ((thearray[i] == "undefined") || (thearray[i] == "") || (thearray[i] == null))
			return i;
		}
	return thearray.length;
}

// Replacement for arrayname.push(value) not implemented in IE until version 5.5
// Appends element to the array
function arraypush(thearray,value) {
	thearray[ getarraysize(thearray) ] = value;
}

// Replacement for arrayname.pop() not implemented in IE until version 5.5
// Removes and returns the last element of an array
function arraypop(thearray) {
	thearraysize = getarraysize(thearray);
	retval = thearray[thearraysize - 1];
	delete thearray[thearraysize - 1];
	return retval;
}


function emoticon(text) {
	var txtarea = document.postform.body;
	text = ' ' + text + ' ';
	if (txtarea.createTextRange && txtarea.caretPos) {
		var caretPos = txtarea.caretPos;
		caretPos.text = caretPos.text.charAt(caretPos.text.length - 1) == ' ' ? text + ' ' : text;
		txtarea.focus();
	} else {
		txtarea.value  += text;
		txtarea.focus();
	}
}


function setstyle(number) {
	var txtarea = document.postform.body;

	donotinsert = false;
	theSelection = false;
	last = 0;

	if (number == -1) { // Close all open tags & default button names
		while (code[0]) {
			cnumber = arraypop(code) - 1;
			txtarea.value += tags[cnumber + 1];
			eval('document.postform.style' + cnumber + '.style.fontWeight =  "normal" ');
		}
		eval('document.postform.stylec.style.fontWeight = "normal" ');
		imageTag = false; // All tags are closed including image tags :D
		txtarea.focus();
		return;
	}

	if (number == -2) {
		var smilestable = document.getElementById('smilestable');
		if(! smilestable) return false; 
		if(smilestable.style.display == 'block') {
			eval('document.postform.styles.style.fontWeight =  "normal" ');
			smilestable.style.display = 'none';
		}
		else {
			eval('document.postform.styles.style.fontWeight =  "bold" ');
			smilestable.style.display = 'block';
		}
		return;
	}

	if ((clientVer >= 4) && is_ie && is_win)
	{
		theSelection = document.selection.createRange().text; // Get text selection
		if (theSelection) {
			// Add tags around selection
			document.selection.createRange().text = tags[number] + theSelection + tags[number+1];
			txtarea.focus();
			theSelection = '';
			return;
		}
	}
	else if (txtarea.selectionEnd && (txtarea.selectionEnd - txtarea.selectionStart > 0))
	{
		mozWrap(txtarea, tags[number], tags[number+1]);
		return;
	}
	
	// Find last occurance of an open tag the same as the one just clicked
	for (i = 0; i < code.length; i++) {
		if (code[i] == number+1) {
			last = i;
			donotinsert = true;
		}
	}

	if (donotinsert) {		// Close all open tags up to the one just clicked & default button names
		while (code[last]) {
			cnumber = arraypop(code) - 1;
			if (txtarea.createTextRange && txtarea.caretPos)
			{
				var caretPos=txtarea.caretPos;
				caretPos.text=(caretPos.text.charAt(caretPos.text.length - 1)==' ') ? tags[cnumber+1] + ' ' : tags[cnumber+1];
			} else
			{
				mozInsert(txtarea, tags[cnumber+1])
			}

			
			eval('document.postform.style' + cnumber + '.style.fontWeight = "normal"');
		}
		imageTag = false;
		if(last == 0) eval('document.postform.stylec.style.fontWeight = "normal"');
		txtarea.focus();
		return;
	} else { // Open tags
	
		if (imageTag && (number != 8)) {		// Close image tag before adding another
			txtarea.value += tags[9];
			lastValue = arraypop(code) - 1;	// Remove the close image tag from the list
			eval('document.postform.style8.style.fontWeight = "normal" ');
			imageTag = false;
		}
		
		// Open tag
		if (txtarea.createTextRange && txtarea.caretPos)
		{
			var caretPos=txtarea.caretPos;
			caretPos.text=(caretPos.text.charAt(caretPos.text.length - 1)==' ') ? tags[number] + ' ' : tags[number];
		} else
		{
			mozInsert(txtarea, tags[number]);
		}
		
		if ((number == 8) && (imageTag == false)) imageTag = 1; // Check to stop additional tags after an unclosed image tag
		arraypush(code,number+1);
		eval('document.postform.style' + number + '.style.fontWeight = "bold" ');
		eval('document.postform.stylec.style.fontWeight = "bold" ');
		txtarea.focus();
		return;
	}
	storeCaret(txtarea);
}

// From http://www.massless.org/mozedit/
function mozWrap(txtarea, open, close)
{
	var selLength = txtarea.textLength;
	var selStart = txtarea.selectionStart;
	var selEnd = txtarea.selectionEnd;
	if (selEnd == 1 || selEnd == 2) 
		selEnd = selLength;

	var s1 = (txtarea.value).substring(0,selStart);
	var s2 = (txtarea.value).substring(selStart, selEnd)
	var s3 = (txtarea.value).substring(selEnd, selLength);
	txtarea.value = s1 + open + s2 + close + s3;
	return;
}

function mozInsert(txtarea, textins)
{
	var es=txtarea.selectionEnd;
	var txt1=txtarea.value.substring(0,es);
	var txt2=txtarea.value.substring(es,txtarea.value.length);
	txtarea.value=txt1+textins+txt2;
	txtarea.selectionEnd=es+textins.length;
}

// Insert at Claret position. Code from
// http://www.faqts.com/knowledge_base/view.phtml/aid/1052/fid/130
function storeCaret(textEl) {
	if (textEl.createTextRange) textEl.caretPos = document.selection.createRange().duplicate();
}

function insert_smile(imgtag) {
	var txtarea = document.postform.body;
	txtarea.value += imgtag;
	txtarea.focus();
	setstyle('-2');
	return;	
}
