<template>
    <div style="height: 100%;width: 100%;">
        <div id="map"></div>
        <!-- info -->
        <div v-show="info.show" style="height: 100%;width: 100%;">
            <div id="title">
                {{info.title}} : {{info.time}}
            </div>
            <div id="info">
                <span style="font-size: 35px;" v-show="info.shows.maxShindo">
                    <center>最大震度{{info.maxShindo}}</center>
                </span>
                <span v-show="info.shows.epicenter">&nbsp;&nbsp;&nbsp;震源地&nbsp;&nbsp;&nbsp;&nbsp;: {{info.epicenter.name}}</span>
                <br>
                <span v-show="info.shows.magnitude">地震の规模&nbsp;: M{{info.magnitude}}</span>
                <br>
                <span v-show="info.shows.depth">地震の深さ&nbsp;: 約{{info.depth}}</span>
            </div>
        </div>
    </div>
</template>

<script>
import jmaPoints from '@/components/js/jmaPoints.json'
import jmaAreas from '@/components/js/jmaAreas.json'
/* ↑ From EarthQuicklyForWeb (ameuma773) */

import axios from 'axios'
import mapboxgl from 'mapbox-gl'
import 'mapbox-gl/dist/mapbox-gl.css'
import diff_match_patch from 'diff-match-patch'

import world from '@/components/geojson/world.json'
import japan_fuken from '@/components/geojson/japan_fuken.json'
import jmaStations from '@/components/js/jmaStations.json'
import '@/components/css/framework.css'
import '@/components/css/marker.css'

let map, popup, epicenterPopup

export default{
    name: 'bousaiApp',
    data(){
        return{
            info: {
                show: true,
                title: '各地の震度',
                time: '',
                epicenter: {
                    name: '',
                    lat: null,
                    lon: null
                },
                depth: '',
                magnitude: '',
                maxShindo: '',
                isTsunami: '',
                shows: {
                    maxShindo: true,
                    epicenter: true,
                    magnitude: true,
                    depth: true
                }
            }
        }
    },
    methods: {
        init(){
            mapboxgl.accessToken = 'pk.eyJ1IjoieW93b3QiLCJhIjoiY2tkNG10ZnpmMTNuYTJwcGJja2w2aTVuYSJ9.9QRtVncQVTbedZZBz3pUeA'
            map = new mapboxgl.Map({
                container: 'map',
                style: {
                    version: 8,
                    sources: {},
                    layers: [
                        {
                        id: "background",
                        type: "background",
                        layout: {},
                        paint: {
                            "background-color": '#355377'
                        },
                        },
                    ],
                },
                center: [139.19, 38.42],
                zoom: 4.41,
                attributionControl: false,
                projection: 'mercator'
            })

            map.on('style.load', () => {
                map.dragRotate.disable()
            })

            map.on('load', () => {
                map.addControl(new mapboxgl.AttributionControl({customAttribution: '<a href="https://projectbs.cn">Project BS</a> | <a href="https://www.jma.go.jp/">気象庁</a> | <a href="https://www.p2pquake.net/">P2P地震情報</a>'}))

                popup = new mapboxgl.Popup({
                    closeButton: false,
                    closeOnClick: false,
                    anchor: 'left',
                    offset: [20, 0]
                })

                epicenterPopup = new mapboxgl.Popup({
                    closeButton: false,
                    closeOnClick: false,
                    anchor: 'left',
                    offset: [25, 0]
                })

                map.addSource('japan_fuken', {
                    type: 'geojson',
                    data: japan_fuken
                })
                map.addSource('world', {
                    type: 'geojson',
                    data: world
                })
                map.addLayer({
                    id: 'japan_fuken_fill',
                    type: 'fill',
                    source: 'japan_fuken',
                    paint: {
                        'fill-color': '#EDEEEC'
                    }
                })
                map.addLayer({
                    id: 'world_fill',
                    type: 'fill',
                    source: 'world',
                    paint: {
                        'fill-color': '#4D617C'
                    }
                })
                map.addLayer({
                    id: 'world_line',
                    type: 'line',
                    source: 'world',
                    paint: {
                        'line-width': 1,
                        'line-color': '#EDEEEC'
                    }
                })
                map.addLayer({
                    id: 'japan_fuken_line',
                    type: 'line',
                    source: 'japan_fuken',
                    paint: {
                        'line-width': 1,
                        'line-color': '#777977'
                    }
                })
                map.easeTo({
                    padding: {
                        left: 210
                    }
                })
                console.log(map.getBounds(japan_fuken))
            })
        },
        getInfo(){
            let maxShindo, shindo
            axios.get(`https://api.p2pquake.net/v2/history?codes=551&limit=100&timestamp=${new Date().getTime()}`).then(data => {
                let json = data.data
                if (json[0].issue.type == 'DetailScale'){
                    this.info.title = '各地の震度'
                    this.info.shows.maxShindo = true
                    this.info.shows.epicenter = true
                    this.info.shows.magnitude = true
                    this.info.shows.depth = true
                    this.info.epicenter.name = json[0].earthquake.hypocenter.name
                    this.info.epicenter.lat = json[0].earthquake.hypocenter.latitude
                    this.info.epicenter.lon = json[0].earthquake.hypocenter.longitude
                    this.info.magnitude = json[0].earthquake.hypocenter.magnitude
                    this.info.magnitude = this.info.magnitude.toFixed(1)
                    this.info.depth = json[0].earthquake.hypocenter.depth
                    this.info.depth = this.info.depth == 0 ? '' : this.info.depth + 'km'
                    let time = new Date(json[0].earthquake.time)
                    let MM = time.getMonth()+1 < 10 ? '' + time.getMonth()+1 : time.getMonth()+1
                    let DD = time.getDate() < 10 ? '0' + time.getDate() : time.getDate()
                    let HH = time.getHours() < 10 ? '0' + time.getHours() : time.getHours()
                    let mm = time.getMinutes() < 10 ? '0' + time.getMinutes() : time.getMinutes()
                    this.info.time = `${MM}月${DD}日 ${HH}:${mm}ごろ`
                    if(json[0].earthquake.domesticTsunami == 'None'){
                        this.info.isTsunami = '津波の心配なし'
                    }else{
                        this.info.isTsunami = '津波情報発表中'
                    }
                    let maxScale = json[0].earthquake.maxScale
                    switch(maxScale){
                        case 10:
                            maxShindo = 1
                            break
                        case 20:
                            maxShindo = 2
                            break
                        case 30:
                            maxShindo = 3
                            break
                        case 40:
                            maxShindo = 4
                            break
                        case 45:
                            maxShindo = "5-"
                            break
                        case 50:
                            maxShindo = "5+"
                            break
                        case 55:
                            maxShindo = "6-"
                            break
                        case 60:
                            maxShindo = "6+"
                            break
                        case 70:
                            maxShindo = "7"
                            break
                    }
                    this.info.maxShindo = maxShindo

                    let marker_epicenter = document.createElement('div')
                    marker_epicenter.className = 'marker_epicenter'
                    new mapboxgl.Marker(marker_epicenter).setLngLat([this.info.epicenter.lon, this.info.epicenter.lat]).addTo(map)

                    marker_epicenter.onmouseover = () => {
                        epicenterPopup.setLngLat([this.info.epicenter.lon, this.info.epicenter.lat]).setHTML(`<span style="position: relative; top: -3px; font-size: 26px;">${this.info.epicenter.name}</span>`).addTo(map)
                    }

                    marker_epicenter.onmouseleave = () => {
                        epicenterPopup.remove()
                    }



                    //暂代fitBounds
                    map.flyTo({
                        center: [this.info.epicenter.lon, this.info.epicenter.lat],
                        essential: true,
                        zoom: 7,
                        duration: 3000
                    })

                    let shindo = []
                    //stations
                    //默认获取最新
                    for(let i=0;i<json[0].points.length;i++){
                        let addr = json[0].points[i].addr
                        let scale = json[0].points[i].scale
                        switch(scale){
                            case 10:
                                shindo[i] = 1
                                break
                            case 20:
                                shindo[i] = 2
                                break
                            case 30:
                                shindo[i] = 3
                                break
                            case 40:
                                shindo[i] = 4
                                break
                            case 45:
                                shindo[i] = "5-"
                                break
                            case 50:
                                shindo[i] = "5+"
                                break
                            case 55:
                                shindo[i] = "6-"
                                break
                            case 60:
                                shindo[i] = "6+"
                                break
                            case 70:
                                shindo[i] = "7"
                                break
                        }
                        try{
                            // addr = this.furigana(addr, this.getJm(addr))
                            // console.log(addr);
                            let lat = jmaStations[jmaPoints.indexOf(addr)]['lat']
                            let lon = jmaStations[jmaPoints.indexOf(addr)]['lon']
                            let addrJm = jmaStations[jmaPoints.indexOf(addr)]['furigana']
                            addr = this.furigana(addr, addrJm)
                            let marker_circle_shindo = document.createElement('div')
                            marker_circle_shindo.className = `marker_circle_shindo${shindo[i]}_${i}`
                            new mapboxgl.Marker(marker_circle_shindo).setLngLat([lon, lat]).addTo(map)
                            marker_circle_shindo.onmouseover = () => {
                                popup.setLngLat([lon, lat]).setHTML(`${addr} 震度${shindo[i]}`).addTo(map)
                            }

                            marker_circle_shindo.onmouseleave = () => {
                                popup.setLngLat([lon, lat]).setHTML(`${addr} 震度${shindo[i]}`).addTo(map)
                                popup.remove()
                            }
                        }catch{
                            //console.log(`Error while generating marker: ${addr} 震度${shindo}`)
                        }
                    }
                }else if(json[0].issue.type == 'Destination'){
                    this.info.title = '震源情報'
                    this.info.shows.maxShindo = true
                    this.info.shows.epicenter = true
                    this.info.shows.magnitude = true
                    this.info.shows.depth = true
                    let time = new Date(json[0].earthquake.time)
                    let MM = time.getMonth()+1 < 10 ? '' + time.getMonth()+1 : time.getMonth()+1
                    let DD = time.getDate() < 10 ? '0' + time.getDate() : time.getDate()
                    let HH = time.getHours() < 10 ? '0' + time.getHours() : time.getHours()
                    let mm = time.getMinutes() < 10 ? '0' + time.getMinutes() : time.getMinutes()
                    this.info.time = `${MM}月${DD}日 ${HH}:${mm}ごろ`
                    this.info.epicenter.name = json[0].earthquake.hypocenter.name
                    this.info.epicenter.lat = json[0].earthquake.hypocenter.latitude
                    this.info.epicenter.lon = json[0].earthquake.hypocenter.longitude
                    this.info.magnitude = json[0].earthquake.hypocenter.magnitude
                    this.info.magnitude = this.info.magnitude.toFixed(1)
                    this.info.depth = json[0].earthquake.hypocenter.depth
                    this.info.depth = this.info.depth == 0 ? '' : this.info.depth + 'km'
                    if(json[0].earthquake.domesticTsunami == 'None'){
                        this.info.isTsunami = '津波の心配なし'
                    }else{
                        this.info.isTsunami = '津波情報発表中'
                    }
                    let maxScale = json[0].earthquake.maxScale
                    switch(maxScale){
                        case 10:
                            maxShindo = 1
                            break
                        case 20:
                            maxShindo = 2
                            break
                        case 30:
                            maxShindo = 3
                            break
                        case 40:
                            maxShindo = 4
                            break
                        case 45:
                            maxShindo = "5-"
                            break
                        case 50:
                            maxShindo = "5+"
                            break
                        case 55:
                            maxShindo = "6-"
                            break
                        case 60:
                            maxShindo = "6+"
                            break
                        case 70:
                            maxShindo = "7"
                            break
                    }
                    this.info.maxShindo = maxShindo
                    let marker_epicenter = document.createElement('div')
                    marker_epicenter.className = 'marker_epicenter'
                    new mapboxgl.Marker(marker_epicenter).setLngLat([this.info.epicenter.lon, this.info.epicenter.lat]).addTo(map)

                    marker_epicenter.onmouseover = () => {
                        epicenterPopup.setLngLat([this.info.epicenter.lon, this.info.epicenter.lat]).setHTML(`<span style="position: relative; top: -3px; font-size: 26px;">${this.info.epicenter.name}</span>`).addTo(map)
                    }

                    marker_epicenter.onmouseleave = () => {
                        epicenterPopup.remove()
                    }
                }else if(json[0].issue.type == 'ScalePrompt'){
                    this.info.title = '震度速報'
                    this.info.shows.maxShindo = true
                    this.info.shows.epicenter = true
                    this.info.shows.magnitude = false
                    this.info.shows.depth = false
                    let time = new Date(json[0].earthquake.time)
                    let MM = time.getMonth()+1 < 10 ? '' + time.getMonth()+1 : time.getMonth()+1
                    let DD = time.getDate() < 10 ? '0' + time.getDate() : time.getDate()
                    let HH = time.getHours() < 10 ? '0' + time.getHours() : time.getHours()
                    let mm = time.getMinutes() < 10 ? '0' + time.getMinutes() : time.getMinutes()
                    this.info.time = `${MM}月${DD}日 ${HH}:${mm}ごろ`
                    this.info.epicenter.name = '調査中'
                    if(json[0].earthquake.domesticTsunami == 'None'){
                        this.info.isTsunami = '津波の心配なし'
                    }else{
                        this.info.isTsunami = '津波情報発表中'
                    }
                    let maxScale = json[0].earthquake.maxScale
                    let shindo
                    for(let i=0;i<json[0].points.length;i++){
                        let addr = json[0].points[i].addr
                        let scale = json[0].points[i].scale
                        switch(scale){
                            case 10:
                                shindo[i] = 1
                                break
                            case 20:
                                shindo[i] = 2
                                break
                            case 30:
                                shindo[i] = 3
                                break
                            case 40:
                                shindo[i] = 4
                                break
                            case 45:
                                shindo[i] = "5-"
                                break
                            case 50:
                                shindo[i] = "5+"
                                break
                            case 55:
                                shindo[i] = "6-"
                                break
                            case 60:
                                shindo[i] = "6+"
                                break
                            case 70:
                                shindo[i] = "7"
                                break
                        }
                        try{
                            // addr = this.furigana(addr, this.getJm(addr))
                            // console.log(addr);
                            let lat = jmaStations[jmaAreas.indexOf(addr)]['lat']
                            let lon = jmaStations[jmaAreas.indexOf(addr)]['lon']
                            let addrJm = jmaStations[jmaAreas.indexOf(addr)]['furigana']
                            addr = this.furigana(addr, addrJm)
                            let marker_circle_shindo = document.createElement('div')
                            marker_circle_shindo.className = `marker_circle_shindo${shindo[i]}_${i}`
                            new mapboxgl.Marker(marker_circle_shindo).setLngLat([lon, lat]).addTo(map)
                            marker_circle_shindo.onmouseover = () => {
                                popup.setLngLat([lon, lat]).setHTML(`${addr} 震度${shindo[i]}`).addTo(map)
                            }

                            marker_circle_shindo.onmouseleave = () => {
                                popup.setLngLat([lon, lat]).setHTML(`${addr} 震度${shindo[i]}`).addTo(map)
                                popup.remove()
                            }
                        }catch{
                            //console.log(`Error while generating marker: ${addr} 震度${shindo}`)
                        }
                    }
                }else if(json[0].issue.type == 'Foreign'){
                    this.info.title = '遠地地震情報'
                    this.info.shows.maxShindo = false
                    this.info.shows.epicenter = true
                    this.info.shows.magnitude = true
                    this.info.shows.depth = false
                    let time = new Date(json[0].earthquake.time)
                    let MM = time.getMonth()+1 < 10 ? '' + time.getMonth()+1 : time.getMonth()+1
                    let DD = time.getDate() < 10 ? '0' + time.getDate() : time.getDate()
                    let HH = time.getHours() < 10 ? '0' + time.getHours() : time.getHours()
                    let mm = time.getMinutes() < 10 ? '0' + time.getMinutes() : time.getMinutes()
                    this.info.time = `${MM}月${DD}日 ${HH}:${mm}ごろ`
                    this.info.epicenter.name = json[0].earthquake.hypocenter.name
                    this.info.epicenter.lat = json[0].earthquake.hypocenter.latitude
                    this.info.epicenter.lon = json[0].earthquake.hypocenter.longitude
                    this.info.magnitude = json[0].earthquake.hypocenter.magnitude
                    this.info.magnitude = this.info.magnitude.toFixed(1)
                    let marker_epicenter = document.createElement('div')
                    marker_epicenter.className = 'marker_epicenter'
                    new mapboxgl.Marker(marker_epicenter).setLngLat([this.info.epicenter.lon, this.info.epicenter.lat]).addTo(map)

                    marker_epicenter.onmouseover = () => {
                        epicenterPopup.setLngLat([this.info.epicenter.lon, this.info.epicenter.lat]).setHTML(`<span style="position: relative; top: -3px; font-size: 26px;">${this.info.epicenter.name}</span>`).addTo(map)
                    }

                    marker_epicenter.onmouseleave = () => {
                        epicenterPopup.remove()
                    }

                    map.flyTo({
                        center: [this.info.epicenter.lon, this.info.epicenter.lat],
                        essential: true,
                        duration: 3000
                    })
                }
            })
        },
        furigana(japanese, hiragana) {
            const diffs = new diff_match_patch().diff_main(japanese, hiragana)
            let html = '', ruby = { furigana: null, text: null }
            diffs.push([0, ''])
            diffs.map(([kind, text]) => {
                if (kind == 0) {
                    if (ruby.furigana || ruby.text) {
                        html += `<ruby>${ruby.text}<rp>(</rp><rt>${ruby.furigana}</rt><rp>)</rp></ruby>`
                        ruby.furigana = null
                        ruby.text = null
                    }
                    html += text
                } else {
                    ruby[kind == 1 ? 'furigana' : 'text'] = text
                }
            })
            return html
        }
    },
    mounted(){
        this.init()
        this.getInfo()
    }
}
</script>