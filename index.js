const now = new Date();
  
const year = now.getFullYear();

const date = now.toISOString().split('T')[0];

const bib = document.getElementById('bibtex');

bib.innerHTML = bib.innerHTML
  .replace(/{{YEAR}}/g, year)
  .replace('{{DATE}}', date);

const footer = document.getElementById('footer');

footer.innerHTML = footer.innerHTML.replace(/{{YEAR}}/g, year);
