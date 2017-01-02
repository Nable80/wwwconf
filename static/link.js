function linkenter(id)
{
    el = document.getElementById('l' + id);
    if (!el)
	return;
    el.style.borderBottom = "1px dashed";
}

function linkleave(id)
{
    el = document.getElementById('l' + id);
    if (!el)
	return;
    el.style.borderBottom = "";
}
