var last;

// in IE trailing newlines in a selection are missed.
// this function return them.
function missed_newlines(e, s)
{
	var t = e.createTextRange();
	var i = 0;
	var res = '';

	e.focus();
	t.moveToBookmark(s.getBookmark());

	// count missing newlines
	while (t.compareEndPoints('StartToEnd', t) && t.compareEndPoints('StartToEnd', e.createTextRange())) {
		if (!t.text.length)
			++i;
		t.moveStart('character', 1);
	}

	while (i--)
		res += '\n';
	return res;
}

function wrap(otag, ctag, allowsubj)
{
        if (!last || !allowsubj && last === document.postform.subject)
                return;
	var e = last;
	e.focus();
	if (document.selection) {
		var s = document.selection.createRange();
		s.text = otag + s.text + missed_newlines(e, s) + ctag;
	} if (typeof e.selectionStart == 'number') {
                var start = e.selectionStart;
                var end = e.selectionEnd;
                var t = e.value;
                var scroll = e.scrollTop;
                e.value = t.substring(0, start) + otag + t.substring(start, end) + ctag + t.substring(end);
                e.selectionStart = start;
                e.selectionEnd = end + otag.length + ctag.length;
                e.scrollTop = scroll;
        } 
}

function show()
{
	var smiles = document.getElementById('smiles');
	if (!smiles)
		return;
	if (smiles.style.display == 'block') {
		document.postform.smile.style.fontWeight = 'normal';
		smiles.style.display = 'none';
	} else {
		document.postform.smile.style.fontWeight = 'bold';
		smiles.style.display = 'block';
	}
}

function insert(text, allowsubj)
{
        if (!last || !allowsubj && last === document.postform.subject)
                return;
        var e = last;
        e.focus();
        if (document.selection)
                document.selection.createRange().text = text;
        else if (typeof e.selectionStart == 'number') {
                var start = e.selectionStart;
                var end = e.selectionEnd;
                var scroll = e.scrollTop;
                e.value = e.value.substring(0, start) + text + e.value.substring(end);
                e.selectionStart = start + text.length;
                e.selectionEnd = start + text.length;
                e.scrollTop = scroll;
        }
}
