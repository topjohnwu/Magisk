package com.topjohnwu.magisk.extensions

import io.reactivex.Observable
import io.reactivex.Scheduler
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers

fun <T> Observable<T>.applySchedulers(
    subscribeOn: Scheduler = Schedulers.io(),
    observeOn: Scheduler = AndroidSchedulers.mainThread()
): Observable<T> = this.subscribeOn(subscribeOn).observeOn(observeOn)

/*=== ALIASES FOR OBSERVABLES ===*/

typealias OnCompleteListener = () -> Unit
typealias OnSuccessListener<T> = (T) -> Unit
typealias OnErrorListener = (Throwable) -> Unit

/*=== ALIASES FOR OBSERVABLES ===*/

fun <T> Observable<T>.subscribeK(
    onError: OnErrorListener = { it.printStackTrace() },
    onComplete: OnCompleteListener = {},
    onNext: OnSuccessListener<T> = {}
) = applySchedulers()
    .subscribe(onNext, onError, onComplete)

