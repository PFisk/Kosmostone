const axios = require('axios');
const express = require('express');
const cheerio = require('cheerio');
const fs = require('fs');

const PORT = 8080;

const app = express();

const currentYear = new Date().getFullYear();

const page = `https://fireball.amsmeteors.org/members/imo_view/browse_events?country=-1&year=${currentYear}&num_report_select=toChange&event=&event_id=&event_year=&num_report=2`;
const mainURL = 'https://fireball.amsmeteors.org';

const getFireballs = async (url) => {
    const response = await axios.get(url);

    const html = response.data;
    const $ = cheerio.load(html);

    const data = {
        fireballs: []
    };

    console.log("Getting data");

    // Create an array of promises for processing each row
    const promises = [];

    $("table.table.table-hover.table-condensed.table-results tbody tr", html).each(function () {
        // Push each processing step as a promise to the array
        promises.push((async () => {
            const ID = $(this).find("td").eq(0).text().trim();
            const eventLink = $(this).find("td").eq(0).find("a").attr("href");
            const reportNum = $(this).find("td").eq(1).text().trim().replace(/\D/g, '');
            const UTDate = $(this).find("td").eq(2).text().trim();
            const country = $(this).find("td").eq(4).text().trim();
            const location = $(this).find("td").eq(5).text().trim();
            const dSoundPositive = $(this).find("td").eq(6).find(".sgraph_number .yes").text().replace(/\D/g, '');
            const dSoundNegative = $(this).find("td").eq(6).find(".sgraph_number .no").text().replace(/\D/g, '');
            const cSoundPositive = $(this).find("td").eq(7).find(".sgraph_number .yes").text().replace(/\D/g, '');
            const cSoundNegative = $(this).find("td").eq(7).find(".sgraph_number .no").text().replace(/\D/g, '');
            const fragPositive = $(this).find("td").eq(8).find(".sgraph_number .yes").text().replace(/\D/g, '');
            const fragNagative = $(this).find("td").eq(8).find(".sgraph_number .no").text().replace(/\D/g, '');

            const eventLinkFormatted = mainURL + eventLink

            console.log("ID: ", ID);

            let cometColor = "";
            if (reportNum) {
                cometColor = await getCometColor(eventLinkFormatted);
                console.log("Consensus color for ID (", ID, "):", cometColor)
            }

            if (reportNum) {
                data.fireballs.push({
                    eventID: ID,
                    eventLink: eventLinkFormatted,
                    reportAmount: reportNum,
                    UTDate: UTDate,
                    country: country,
                    location: location,
                    color: cometColor,
                    delayedSoundPositive: dSoundPositive,
                    delayedSoundNegative: dSoundNegative,
                    concurrentSoundPositive: cSoundPositive,
                    concurrentSoundNegative: cSoundNegative,
                    fragmentationPositive: fragPositive,
                    fragmentationNegative: fragNagative
                });
            }
        })());
    });

    // Wait for all promises to resolve
    await Promise.all(promises);

    return data;
};

async function getCometColor(url) {
    const eventReportLinks = await getEventReports(url);

    // Map eventReportLinks to promises for fetching color data
    const colorReportPromises = eventReportLinks.map(async (e) => {
        return await getColorData(e);
    });

    // Wait for all color data to resolve
    const colorReports = await Promise.all(colorReportPromises);

    // Get consensus color from the resolved colorReports
    const consensusColor = getColorConsensus(colorReports);

    return consensusColor;
}

function getColorConsensus(colorReports) {
    // Split the comma-separated strings into individual colors
    const allColors = colorReports.flatMap(report => report.split(',').map(color => color.trim()));

    // Count occurrences of each color
    const colorCounts = allColors.reduce((counts, color) => {
        counts[color] = (counts[color] || 0) + 1;
        return counts;
    }, {});

    // Find the most common color
    let mostCommon = null;
    let maxCount = 0;

    for (const [color, count] of Object.entries(colorCounts)) {
        if (count > maxCount) {
            maxCount = count;
            mostCommon = color;
        }
    }

    return mostCommon;
}

async function getEventReports(url) {
    const response = await axios.get(url);
    const $ = cheerio.load(response.data);

    const links = [];

    $("table.table-hover.table-condensed.table-results tbody tr", response.data).each(function(){
        const eventReportLink = $(this).find("td").eq(0).find("a").attr("href")
        links.push(mainURL + eventReportLink);
    })

    return links;
}

async function getColorData(url) {
    const response = await axios.get(url);
    const $ = cheerio.load(response.data);
    const color = $('th:contains("Color")', response.data).next('td').text().trim();

    return color;
}

const scrape = async () => {
    const data = await getFireballs(page);

    fs.writeFile('./data/fireballs.json', JSON.stringify(data), "utf-8", function(err) { console.log(err) })

    console.log("All done!")
}

scrape()