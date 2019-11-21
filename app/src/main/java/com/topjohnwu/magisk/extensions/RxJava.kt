package com.topjohnwu.magisk.extensions

import androidx.databinding.ObservableField
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.superuser.internal.UiThreadHandler
import io.reactivex.*
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.disposables.Disposables
import io.reactivex.functions.BiFunction
import io.reactivex.schedulers.Schedulers
import androidx.databinding.Observable as BindingObservable

fun <T> Observable<T>.applySchedulers(
    subscribeOn: Scheduler = Schedulers.io(),
    observeOn: Scheduler = AndroidSchedulers.mainThread()
): Observable<T> = this.subscribeOn(subscribeOn).observeOn(observeOn)

fun <T> Flowable<T>.applySchedulers(
    subscribeOn: Scheduler = Schedulers.io(),
    observeOn: Scheduler = AndroidSchedulers.mainThread()
): Flowable<T> = this.subscribeOn(subscribeOn).observeOn(observeOn)

fun <T> Single<T>.applySchedulers(
    subscribeOn: Scheduler = Schedulers.io(),
    observeOn: Scheduler = AndroidSchedulers.mainThread()
): Single<T> = this.subscribeOn(subscribeOn).observeOn(observeOn)

fun <T> Maybe<T>.applySchedulers(
    subscribeOn: Scheduler = Schedulers.io(),
    observeOn: Scheduler = AndroidSchedulers.mainThread()
): Maybe<T> = this.subscribeOn(subscribeOn).observeOn(observeOn)

fun Completable.applySchedulers(
    subscribeOn: Scheduler = Schedulers.io(),
    observeOn: Scheduler = AndroidSchedulers.mainThread()
): Completable = this.subscribeOn(subscribeOn).observeOn(observeOn)

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

fun <T> Single<T>.subscribeK(
    onError: OnErrorListener = { it.printStackTrace() },
    onNext: OnSuccessListener<T> = {}
) = applySchedulers()
    .subscribe(onNext, onError)

fun <T> Maybe<T>.subscribeK(
    onError: OnErrorListener = { it.printStackTrace() },
    onComplete: OnCompleteListener = {},
    onSuccess: OnSuccessListener<T> = {}
) = applySchedulers()
    .subscribe(onSuccess, onError, onComplete)

fun <T> Flowable<T>.subscribeK(
    onError: OnErrorListener = { it.printStackTrace() },
    onComplete: OnCompleteListener = {},
    onNext: OnSuccessListener<T> = {}
) = applySchedulers()
    .subscribe(onNext, onError, onComplete)

fun Completable.subscribeK(
    onError: OnErrorListener = { it.printStackTrace() },
    onComplete: OnCompleteListener = {}
) = applySchedulers()
    .subscribe(onComplete, onError)


fun <T> Observable<out T>.updateBy(
    field: KObservableField<T?>
) = doOnNextUi { field.value = it }
    .doOnErrorUi { field.value = null }

fun <T> Single<out T>.updateBy(
    field: KObservableField<T?>
) = doOnSuccessUi { field.value = it }
    .doOnErrorUi { field.value = null }

fun <T> Maybe<out T>.updateBy(
    field: KObservableField<T?>
) = doOnSuccessUi { field.value = it }
    .doOnErrorUi { field.value = null }
    .doOnComplete { field.value = field.value }

fun <T> Flowable<out T>.updateBy(
    field: KObservableField<T?>
) = doOnNextUi { field.value = it }
    .doOnErrorUi { field.value = null }

fun Completable.updateBy(
    field: KObservableField<Boolean>
) = doOnCompleteUi { field.value = true }
    .doOnErrorUi { field.value = false }


fun <T> Observable<T>.doOnSubscribeUi(body: () -> Unit) =
    doOnSubscribe { UiThreadHandler.run { body() } }

fun <T> Single<T>.doOnSubscribeUi(body: () -> Unit) =
    doOnSubscribe { UiThreadHandler.run { body() } }

fun <T> Maybe<T>.doOnSubscribeUi(body: () -> Unit) =
    doOnSubscribe { UiThreadHandler.run { body() } }

fun <T> Flowable<T>.doOnSubscribeUi(body: () -> Unit) =
    doOnSubscribe { UiThreadHandler.run { body() } }

fun Completable.doOnSubscribeUi(body: () -> Unit) =
    doOnSubscribe { UiThreadHandler.run { body() } }


fun <T> Observable<T>.doOnErrorUi(body: (Throwable) -> Unit) =
    doOnError { UiThreadHandler.run { body(it) } }

fun <T> Single<T>.doOnErrorUi(body: (Throwable) -> Unit) =
    doOnError { UiThreadHandler.run { body(it) } }

fun <T> Maybe<T>.doOnErrorUi(body: (Throwable) -> Unit) =
    doOnError { UiThreadHandler.run { body(it) } }

fun <T> Flowable<T>.doOnErrorUi(body: (Throwable) -> Unit) =
    doOnError { UiThreadHandler.run { body(it) } }

fun Completable.doOnErrorUi(body: (Throwable) -> Unit) =
    doOnError { UiThreadHandler.run { body(it) } }


fun <T> Observable<T>.doOnNextUi(body: (T) -> Unit) =
    doOnNext { UiThreadHandler.run { body(it) } }

fun <T> Flowable<T>.doOnNextUi(body: (T) -> Unit) =
    doOnNext { UiThreadHandler.run { body(it) } }

fun <T> Single<T>.doOnSuccessUi(body: (T) -> Unit) =
    doOnSuccess { UiThreadHandler.run { body(it) } }

fun <T> Maybe<T>.doOnSuccessUi(body: (T) -> Unit) =
    doOnSuccess { UiThreadHandler.run { body(it) } }

fun <T> Maybe<T>.doOnCompleteUi(body: () -> Unit) =
    doOnComplete { UiThreadHandler.run { body() } }

fun Completable.doOnCompleteUi(body: () -> Unit) =
    doOnComplete { UiThreadHandler.run { body() } }


fun <T, R> Observable<List<T>>.mapList(
    transformer: (T) -> R
) = flatMapIterable { it }
    .map(transformer)
    .toList()

fun <T, R> Single<List<T>>.mapList(
    transformer: (T) -> R
) = flattenAsFlowable { it }
    .map(transformer)
    .toList()

fun <T, R> Maybe<List<T>>.mapList(
    transformer: (T) -> R
) = flattenAsFlowable { it }
    .map(transformer)
    .toList()

fun <T, R> Flowable<List<T>>.mapList(
    transformer: (T) -> R
) = flatMapIterable { it }
    .map(transformer)
    .toList()

fun <T> ObservableField<T>.toObservable(): Observable<T> {
    val observableField = this
    return Observable.create { emitter ->
        observableField.get()?.let { emitter.onNext(it) }

        val callback = object : BindingObservable.OnPropertyChangedCallback() {
            override fun onPropertyChanged(sender: BindingObservable?, propertyId: Int) {
                observableField.get()?.let { emitter.onNext(it) }
            }
        }
        observableField.addOnPropertyChangedCallback(callback)
        emitter.setDisposable(Disposables.fromAction {
            observableField.removeOnPropertyChangedCallback(callback)
        })
    }
}

fun <T : Any> T.toSingle() = Single.just(this)

inline fun <T1, T2, R> zip(
    t1: Single<T1>,
    t2: Single<T2>,
    crossinline zipper: (T1, T2) -> R
) = Single.zip(t1, t2, BiFunction<T1, T2, R> { rt1, rt2 -> zipper(rt1, rt2) })