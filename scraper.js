const axios = require('axios');
const express = require('express');
const cheerio = require('cheerio');
const fs = require('fs');

const PORT = 8080;

const app = express();

const currentYear = new Date().getFullYear();

const page = `https://fireball.amsmeteors.org/members/imo_view/browse_events?country=-1&year=${currentYear}&num_report_select=toChange&event=&event_id=&event_year=&num_report=2`;

const getFireballs = async (url) => {

    const response = await axios.get(url);

    const html = response.data
    const $ = cheerio.load(html)

    const data = {
        fireballs: []
    }

    console.log("Getting dem balls")

    $("table.table.table-hover.table-condensed.table-results tr", html).each(function() {

        const ID = $(this).find("td").eq(0).text().trim()
        const reportNum = $(this).find("td").eq(1).text().trim().replace(/\D/g,'')
        const UTDate = $(this).find("td").eq(2).text().trim()
        const country = $(this).find("td").eq(4).text().trim()
        const location = $(this).find("td").eq(5).text().trim()
        const dSoundPositive = $(this).find("td").eq(6).find(".sgraph_number .yes").text().replace(/\D/g,'')
        const dSoundNegative = $(this).find("td").eq(6).find(".sgraph_number .no").text().replace(/\D/g,'')
        const cSoundPositive = $(this).find("td").eq(7).find(".sgraph_number .yes").text().replace(/\D/g,'')
        const cSoundNegative = $(this).find("td").eq(7).find(".sgraph_number .no").text().replace(/\D/g,'')
        const fragPositive = $(this).find("td").eq(8).find(".sgraph_number .yes").text().replace(/\D/g,'')
        const fragNagative = $(this).find("td").eq(8).find(".sgraph_number .no").text().replace(/\D/g,'')

        console.log("ID", ID)
        
        if (reportNum) {
            data.fireballs.push({
                eventID: ID,
                reportAmount: reportNum,
                UTDate: UTDate,
                country: country,
                location: location,
                delayedSoundPositive: dSoundPositive,
                delayedSoundNegative: dSoundNegative,
                concurrentSoundPositive: cSoundPositive,
                concurrentSoundNegative: cSoundNegative,
                fragmentationPositive: fragPositive,
                fragmentationNegative: fragNagative
            })
        }
    })

    return data

}

const handleizer = (str) => {
    return str.replace(/(\s-\s)|\s/g, '-').replace(/\./g, '')
}

const normalizeString = (str) => {
    return str.normalize("NFD").replace(/[\u0300-\u036f]/g, "")
}

const scrape = async () => {
    const data = await getFireballs(page)

    fs.writeFile('./data/fireballs.json', JSON.stringify(data), "utf-8", function(err) { console.log(err) })

    console.log("All done!")
}

scrape()