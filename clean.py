#!/usr/bin/env python
# # -*- coding: utf-8 -*-

from __future__ import print_function
import urllib2
import xml.etree.ElementTree as ET

def urlopen(path):
    return urllib2.urlopen('http://x.mipt.cc/?' + path)

def vitopen(path):
    r = urllib2.Request('http://x.mipt.cc/?' + path)
    r.add_header('Cookie', 'RTBB=name=Vit|lsel=2|tc=100|tt=1|tv=8|ss=1|lm=132507|fm=132530|lt=37371|ft=37394|dsm=18|seq=cacdd71a98859ace|topics=524287|lann=0|tovr=0|tz=4&')
    return urllib2.urlopen(r)


def root(path, tag):
    r = ET.parse(urlopen(path)).getroot()
    if r.tag != tag:
        raise Exception('root not {} but {}'.format(tag, r.tag))
    return r

def last():
    #return 101740
    return int(root('xmllast', 'lastMessageNumber').text)

dl_rng = 100

def msgs():
    for to in range(last(), 0, -dl_rng):
        fr = to - dl_rng + 1
        if fr < 1:
            fr = 1
        for m in reversed(root('xmlindex&from={}&to={}'.format(fr, to), 'messages')):
            print('get {}'.format(m.get('id')))
            yield m

def getv(m, k):
    #ET.dump(m)
    #print(k)
    return reduce(lambda el, tag: el.find(tag), k, m).text

def check(m, kvs):
    for kv in kvs:
        if check1(m, kv):
            return True
    return False

def check1(m, kv):
    for k,v in kv.items():
        if getv(m, k) != v:
            return False
    return True

def sel_msgs(kvs):
    non = 0
    for m in msgs():
        if non >= 200:
            return
        if m.find('status') is not None:
            if m.find('status').text == 'hided':
                delete(m)
            non = 0
            #break
            continue
        if check(m, kvs):
            non = 0
            yield m
        else:
            non += 1

def hide(m):
    mid = m.get('id')
    print('hide', mid)
    vitopen('hide={}'.format(mid))

def delete(m):
    mid = m.get('id')
    print('del', mid)
    vitopen('delmsg={}'.format(mid))

kvs = [
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'infong902.lxa.perfora.net',
        ('content', 'title'): '...................................................................................................' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'infong902.lxa.perfora.net',
        ('content', 'title'): '___________________________________________________________________________________________________' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'infong902.lxa.perfora.net',
        ('content', 'title'): '~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'infong902.lxa.perfora.net',
        ('content', 'title'): '/|\\\\/|\\\\/|\\\\' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'infong902.lxa.perfora.net',
        ('content', 'title'): '\\\\|/\\\\|/\\\\|/' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'infong902.lxa.perfora.net',
        ('content', 'title'): u'Вайпайте уже хаб борду СУКИИ' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'infong902.lxa.perfora.net',
        ('content', 'title'): u'В АНАЛ АТЛИЧНА' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'infong902.lxa.perfora.net',
        ('content', 'title'): u'ДОСТАТОЧНО???!' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'p3nlhg1093.shr.prod.phx3.secureserver.net',
        ('content', 'title'): u'Жава, ХЛЕБНИ ГОВНЕЦА АРОМАТНОГО' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'p3nlhg1093.shr.prod.phx3.secureserver.net',
        ('content', 'title'): u'В атаку на хупту !!!1111!!!111111111111!!!1 Вит научит тебя любить родину' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'p3nlhg1093.shr.prod.phx3.secureserver.net',
        ('content', 'title'): u'ХЛЕБНИ ГОВНА' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'p3nlhg1093.shr.prod.phx3.secureserver.net',
        ('content', 'title'): u'ВИТ НАУЧИТ ТЕБЯ ЛЮБИТЬ РОДИНУ' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'p3nlhg1093.shr.prod.phx3.secureserver.net',
        ('content', 'title'): u'КТО ВЫ?! КТО ВЫ ЁБ ТВОЮ МАТЬ БЛЯТЬ!?' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'p3nlhg1093.shr.prod.phx3.secureserver.net',
        ('content', 'title'): u'В атаку !!!1111!!!111111111111!!!1' },
    {
        ('author', 'name'): u'Проверь-ка свой анал',
        ('author', 'host'): 'p3nlhg1094.shr.prod.phx3.secureserver.net',
        ('content', 'title'): u'В атаку !!!1111!!!111111111111!!!1' },
    {
        ('author', 'name'): u'Шифратор',
        ('author', 'host'): 'p3nlhg1094.shr.prod.phx3.secureserver.net',
        ('content', 'title'): u'Петушки, в атаку на хупту!!!!11!!' },
    {
        ('author', 'name'): u'Шифратор',
        ('author', 'host'): 'da4.ratemyserver.net',
        ('content', 'title'): u'Петушки, в атаку на хупту!!!!11!!' }
    ]

for m in sel_msgs(kvs):
    hide(m)
