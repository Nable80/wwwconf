// from http://ru.wikipedia.org/wiki/XMLHttpRequest
function createreq() {
        if (typeof XMLHttpRequest === 'undefined') {
                XMLHttpRequest = function() {
                        try { return new ActiveXObject("Msxml2.XMLHTTP.6.0"); }
                        catch(e) {}
                        try { return new ActiveXObject("Msxml2.XMLHTTP.3.0"); }
                        catch(e) {}
                        try { return new ActiveXObject("Msxml2.XMLHTTP"); }
                        catch(e) {}
                        try { return new ActiveXObject("Microsoft.XMLHTTP"); }
                        catch(e) {}
                        throw new Error("This browser does not support XMLHttpRequest.");
                };
        }
        return new XMLHttpRequest();
}

function doshowbody(body, req)
{
        var content;
        if (req.readyState != 4)
                return;
        if (req.status != 200) {
                body.className = 'ierror';
                body.innerHTML = 'Error: status ' + req.status + ': ' + req.statusText;
                return;
        }
        if ( !(content = req.responseXML.getElementsByTagName('body').item(0).firstChild.data)) {
                body.className = 'ierror';
                body.innerHTML = 'Error: bad answer';
                return;
        }
        
        body.className = 'ibody';
        body.innerHTML = content;
}

function showbody(id)
{
        var body, req, e, br, next;

        if (body = document.getElementById('b' + id)) {
                body.parentNode.removeChild(body);
                return;
        }

        br = document.createElement('br');

        body = document.createElement('span');
        body.id = 'b' + id;
        body.className = 'iload';
        body.innerHTML = 'loading...';

        e = document.getElementById('m' + id);
        next = e.nextSibling;
        e.parentNode.insertBefore(br, next);
        e.parentNode.insertBefore(body, next);

        req = createreq();
        req.open('GET', '?xmlbody=' + id, true);
        req.onreadystatechange = function(){doshowbody(body, req);};
        req.send(null);
}
